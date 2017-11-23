//
// Created by jonas on 11/6/17.
//

#include "../include/iCub/TensorFlowInference.h"

// These are all common classes it's handy to reference with no namespace.
using tensorflow::Flag;
using tensorflow::Tensor;
using tensorflow::Status;
using tensorflow::string;
using tensorflow::int32;
using tensorflow::TensorShape;


TensorFlowInference::TensorFlowInference(std::string t_pathGraph, std::string t_pathLabels,  std::string t_model_name) {

    this->pathToGraph = t_pathGraph;
    this->pathToLabels = t_pathLabels;

    this->input_layer = "Mul";
    this->output_layer = "final_result";
    model_name = t_model_name;


}


/************************************* CORE FUNCTIONS  *************************************/


Status TensorFlowInference::ReadLabelsFile(const string &file_name, std::vector<string> *result,
                                           size_t *found_label_count) {
    std::ifstream file(file_name);
    if (!file) {
        return tensorflow::errors::NotFound("Labels file ", file_name,
                                            " not found.");
    }
    result->clear();
    string line;
    while (std::getline(file, line)) {
        result->push_back(line);
    }
    *found_label_count = result->size();
    const int padding = 16;
    while (result->size() % padding) {
        result->emplace_back();
    }
    return Status::OK();
}

tensorflow::Tensor TensorFlowInference::MatToTensor(cv::Mat inputImage) {




    // allocate a Tensor
    Tensor tensorImage(tensorflow::DT_FLOAT, TensorShape({1, this->input_height, this->input_width, 3}));

    // get pointer to memory for that Tensor
    float *p = tensorImage.flat<float>().data();
    // create a "fake" cv::Mat from it
    cv::Mat matToTensor(this->input_height, this->input_width, CV_32FC3, p);

    // Resize inputMat image
    cv::resize(inputImage, inputImage, cv::Size(input_height, input_width));


    // use it here as a destination
    inputImage.convertTo(matToTensor, CV_32FC3);


    normalizeTensor(&tensorImage);


    return tensorImage;
}

tensorflow::Status TensorFlowInference::LoadGraph(const std::string &graph_file_name, std::unique_ptr<tensorflow::Session> *session) {
    tensorflow::GraphDef graph_def;
    Status load_graph_status =
            ReadBinaryProto(tensorflow::Env::Default(), graph_file_name, &graph_def);
    if (!load_graph_status.ok()) {
        return tensorflow::errors::NotFound("Failed to load compute graph at '",
                                            graph_file_name, "'");
    }
    session->reset(tensorflow::NewSession(tensorflow::SessionOptions()));
    Status session_create_status = (*session)->Create(graph_def);
    if (!session_create_status.ok()) {
        return session_create_status;
    }
    return Status::OK();
}

tensorflow::Status TensorFlowInference::GetTopLabels(const std::vector<Tensor> &outputs, int how_many_labels, Tensor *indices,
                                  Tensor *scores) {
    auto root = tensorflow::Scope::NewRootScope();
    using namespace ::tensorflow::ops;  // NOLINT(build/namespaces)

    string output_name = "top_k";
    TopK(root.WithOpName(output_name), outputs[0], how_many_labels);
    // This runs the GraphDef network definition that we've just constructed, and
    // returns the results in the output tensors.
    tensorflow::GraphDef graph;
    TF_RETURN_IF_ERROR(root.ToGraphDef(&graph));

    std::unique_ptr<tensorflow::Session> session(
            tensorflow::NewSession(tensorflow::SessionOptions()));
    TF_RETURN_IF_ERROR(session->Create(graph));
    // The TopK node returns two outputs, the scores and their original indices,
    // so we have to append :0 and :1 to specify them both.
    std::vector<Tensor> out_tensors;
    TF_RETURN_IF_ERROR(session->Run({}, {output_name + ":0", output_name + ":1"},
                                    {}, &out_tensors));
    *scores = out_tensors[0];
    *indices = out_tensors[1];
    return Status::OK();
}

tensorflow::Status TensorFlowInference::PrintTopLabels(const std::vector<tensorflow::Tensor> &outputs,
                                                       const std::string &labels_file_name) {
    std::vector<string> labels;
    size_t label_count;
    Status read_labels_status =
            ReadLabelsFile(labels_file_name, &labels, &label_count);
    if (!read_labels_status.ok()) {
        LOG(ERROR) << read_labels_status;
        return read_labels_status;
    }
    const int how_many_labels = std::min(5, static_cast<int>(label_count));
    Tensor indices;
    Tensor scores;
    TF_RETURN_IF_ERROR(GetTopLabels(outputs, how_many_labels, &indices, &scores));
    tensorflow::TTypes<float>::Flat scores_flat = scores.flat<float>();
    tensorflow::TTypes<int32>::Flat indices_flat = indices.flat<int32>();
    for (int pos = 0; pos < how_many_labels; ++pos) {
        const int label_index = indices_flat(pos);
        const float score = scores_flat(pos);
        LOG(INFO) << labels[label_index] << " (" << label_index << "): " << score;
    }
    return Status::OK();
}



std::string  TensorFlowInference::GetTopClass(const std::vector<tensorflow::Tensor> &outputs,
                                                       const std::string &labels_file_name) {
    std::vector<string> labels;
    size_t label_count;
    Status read_labels_status =
            ReadLabelsFile(labels_file_name, &labels, &label_count);
    if (!read_labels_status.ok()) {
        LOG(ERROR) << read_labels_status;
        return "";
    }
    const int how_many_labels = std::min(5, static_cast<int>(label_count));
    Tensor indices;
    Tensor scores;
    GetTopLabels(outputs, how_many_labels, &indices, &scores);
    tensorflow::TTypes<float>::Flat scores_flat = scores.flat<float>();
    tensorflow::TTypes<int32>::Flat indices_flat = indices.flat<int32>();

    return labels[indices_flat(0)];
}


tensorflow::Status
TensorFlowInference::normalizeTensor(tensorflow::Tensor *inputTensor) {

    auto root = tensorflow::Scope::NewRootScope();
    using namespace ::tensorflow::ops;  // NOLINT(build/namespaces)

    string output_name = "normalized";
    std::vector<Tensor> out_tensors;

    std::vector<std::pair<string, tensorflow::Tensor>> inputs = {
            {"input", *inputTensor},
    };


    auto dims_expander = ExpandDims(root, *inputTensor, 0);
    // Bilinearly resize the image to fit the required dimensions.
    auto resized = ResizeBilinear(
            root, dims_expander,
            Const(root.WithOpName("size"), {input_height, input_width}));
    // Subtract the mean and divide by the scale.
    Div(root.WithOpName(output_name), Sub(root, resized, {input_mean}),
        {input_std});

    // This runs the GraphDef network definition that we've just constructed, and
    // returns the results in the output tensor.
    tensorflow::GraphDef graph;


    TF_RETURN_IF_ERROR(root.ToGraphDef(&graph));

    std::unique_ptr<tensorflow::Session> session(
            tensorflow::NewSession(tensorflow::SessionOptions()));
    TF_RETURN_IF_ERROR(session->Create(graph));
    TF_RETURN_IF_ERROR(session->Run({inputs}, {output_name}, {}, &out_tensors));


    *inputTensor = out_tensors[0];

    return Status::OK();

}

std::string TensorFlowInference::inferObject(cv::Mat inputImage) {

    const Tensor resized_tensor = MatToTensor(inputImage);
    std::vector<Tensor> outputs;


    Status run_status = session->Run({{input_layer, resized_tensor}},
                                     {output_layer}, {}, &outputs);

    if (!run_status.ok()) {
        LOG(ERROR) << "Running model failed: " << run_status;
        return "";
    } else {
        PrintTopLabels(outputs, this->pathToLabels);
        return GetTopClass(outputs, this->pathToLabels);

    }

}

tensorflow::Status TensorFlowInference::initGraph() {

    if(!initPreprocessParameters(model_name)){
        return Status(tensorflow::error::FAILED_PRECONDITION, "Unable to compute the model architecture, check the model_name parameters");

    }

    Status load_graph_status = LoadGraph(this->pathToGraph, &session);

    if (!load_graph_status.ok()) {
        LOG(ERROR) << load_graph_status;
        return Status(tensorflow::error::FAILED_PRECONDITION,"Unable to initialize the graph, check the graph and labels path");
    }


    return Status::OK();

}

bool TensorFlowInference::initPreprocessParameters(std::string modelName) {

    if(modelName=="inception_v3"){
        this->input_width = 299;
        this->input_height = 299;

        this->input_mean = 128;
        this->input_std = 128;

    }

    else if (modelName.find("mobilenet") != std::string::npos) {

        this->input_mean = 127.5;
        this->input_std = 127.5;

        if(modelName.find("224") != std::string::npos){
            this->input_width = 224;
            this->input_height = 224;
        }

        else if (modelName.find("192") != std::string::npos){
            this->input_width = 192;
            this->input_height = 192;
        }
        else if (modelName.find("160") != std::string::npos){
            this->input_width = 168;
            this->input_height = 168;
        }
        else if (modelName.find("128") != std::string::npos){
            this->input_width = 128;
            this->input_height = 128;
        }

        else{
            return false;
        }
    }

    else{
        return false;
    }

    return true;
}




