/*
 * Copyright (C) 2006-2019 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

// This is an automatically generated file.

// Generated from the following "Sound" msg definition:
//   float64 time
//   int16 n_samples
//   int16 n_channels
//   int16    n_frequency
//   int16[] l_channel_data
//   int16[] r_channel_data
//   
// Instances of this class can be read and written with YARP ports,
// using a ROS-compatible format.

#ifndef YARP_ROSMSG_Sound_h
#define YARP_ROSMSG_Sound_h

#include <yarp/os/Wire.h>
#include <yarp/os/Type.h>
#include <yarp/os/idl/WireTypes.h>
#include <string>
#include <vector>

namespace yarp {
namespace rosmsg {

class Sound : public yarp::os::idl::WirePortable
{
public:
    yarp::conf::float64_t time;
    std::int16_t n_samples;
    std::int16_t n_channels;
    std::int16_t n_frequency;
    std::vector<std::int16_t> l_channel_data;
    std::vector<std::int16_t> r_channel_data;

    Sound() :
            time(0.0),
            n_samples(0),
            n_channels(0),
            n_frequency(0),
            l_channel_data(),
            r_channel_data()
    {
    }

    void clear()
    {
        // *** time ***
        time = 0.0;

        // *** n_samples ***
        n_samples = 0;

        // *** n_channels ***
        n_channels = 0;

        // *** n_frequency ***
        n_frequency = 0;

        // *** l_channel_data ***
        l_channel_data.clear();

        // *** r_channel_data ***
        r_channel_data.clear();
    }

    bool readBare(yarp::os::ConnectionReader& connection) override
    {
        // *** time ***
        time = connection.expectFloat64();

        // *** n_samples ***
        n_samples = connection.expectInt16();

        // *** n_channels ***
        n_channels = connection.expectInt16();

        // *** n_frequency ***
        n_frequency = connection.expectInt16();

        // *** l_channel_data ***
        int len = connection.expectInt32();
        l_channel_data.resize(len);
        if (len > 0 && !connection.expectBlock((char*)&l_channel_data[0], sizeof(std::int16_t)*len)) {
            return false;
        }

        // *** r_channel_data ***
        len = connection.expectInt32();
        r_channel_data.resize(len);
        if (len > 0 && !connection.expectBlock((char*)&r_channel_data[0], sizeof(std::int16_t)*len)) {
            return false;
        }

        return !connection.isError();
    }

    bool readBottle(yarp::os::ConnectionReader& connection) override
    {
        connection.convertTextMode();
        yarp::os::idl::WireReader reader(connection);
        if (!reader.readListHeader(6)) {
            return false;
        }

        // *** time ***
        time = reader.expectFloat64();

        // *** n_samples ***
        n_samples = reader.expectInt16();

        // *** n_channels ***
        n_channels = reader.expectInt16();

        // *** n_frequency ***
        n_frequency = reader.expectInt16();

        // *** l_channel_data ***
        if (connection.expectInt32() != (BOTTLE_TAG_LIST|BOTTLE_TAG_INT16)) {
            return false;
        }
        int len = connection.expectInt32();
        l_channel_data.resize(len);
        for (int i=0; i<len; i++) {
            l_channel_data[i] = (std::int16_t)connection.expectInt16();
        }

        // *** r_channel_data ***
        if (connection.expectInt32() != (BOTTLE_TAG_LIST|BOTTLE_TAG_INT16)) {
            return false;
        }
        len = connection.expectInt32();
        r_channel_data.resize(len);
        for (int i=0; i<len; i++) {
            r_channel_data[i] = (std::int16_t)connection.expectInt16();
        }

        return !connection.isError();
    }

    using yarp::os::idl::WirePortable::read;
    bool read(yarp::os::ConnectionReader& connection) override
    {
        return (connection.isBareMode() ? readBare(connection)
                                        : readBottle(connection));
    }

    bool writeBare(yarp::os::ConnectionWriter& connection) const override
    {
        // *** time ***
        connection.appendFloat64(time);

        // *** n_samples ***
        connection.appendInt16(n_samples);

        // *** n_channels ***
        connection.appendInt16(n_channels);

        // *** n_frequency ***
        connection.appendInt16(n_frequency);

        // *** l_channel_data ***
        connection.appendInt32(l_channel_data.size());
        if (l_channel_data.size()>0) {
            connection.appendExternalBlock((char*)&l_channel_data[0], sizeof(std::int16_t)*l_channel_data.size());
        }

        // *** r_channel_data ***
        connection.appendInt32(r_channel_data.size());
        if (r_channel_data.size()>0) {
            connection.appendExternalBlock((char*)&r_channel_data[0], sizeof(std::int16_t)*r_channel_data.size());
        }

        return !connection.isError();
    }

    bool writeBottle(yarp::os::ConnectionWriter& connection) const override
    {
        connection.appendInt32(BOTTLE_TAG_LIST);
        connection.appendInt32(6);

        // *** time ***
        connection.appendInt32(BOTTLE_TAG_FLOAT64);
        connection.appendFloat64(time);

        // *** n_samples ***
        connection.appendInt32(BOTTLE_TAG_INT16);
        connection.appendInt16(n_samples);

        // *** n_channels ***
        connection.appendInt32(BOTTLE_TAG_INT16);
        connection.appendInt16(n_channels);

        // *** n_frequency ***
        connection.appendInt32(BOTTLE_TAG_INT16);
        connection.appendInt16(n_frequency);

        // *** l_channel_data ***
        connection.appendInt32(BOTTLE_TAG_LIST|BOTTLE_TAG_INT16);
        connection.appendInt32(l_channel_data.size());
        for (size_t i=0; i<l_channel_data.size(); i++) {
            connection.appendInt16(l_channel_data[i]);
        }

        // *** r_channel_data ***
        connection.appendInt32(BOTTLE_TAG_LIST|BOTTLE_TAG_INT16);
        connection.appendInt32(r_channel_data.size());
        for (size_t i=0; i<r_channel_data.size(); i++) {
            connection.appendInt16(r_channel_data[i]);
        }

        connection.convertTextMode();
        return !connection.isError();
    }

    using yarp::os::idl::WirePortable::write;
    bool write(yarp::os::ConnectionWriter& connection) const override
    {
        return (connection.isBareMode() ? writeBare(connection)
                                        : writeBottle(connection));
    }

    // This class will serialize ROS style or YARP style depending on protocol.
    // If you need to force a serialization style, use one of these classes:
    typedef yarp::os::idl::BareStyle<yarp::rosmsg::Sound> rosStyle;
    typedef yarp::os::idl::BottleStyle<yarp::rosmsg::Sound> bottleStyle;

    // The name for this message, ROS will need this
    static constexpr const char* typeName = "Sound";

    // The checksum for this message, ROS will need this
    static constexpr const char* typeChecksum = "455ee9abb1c807ca3f5f1a7345999324";

    // The source text for this message, ROS will need this
    static constexpr const char* typeText = "\
float64 time\n\
int16 n_samples\n\
int16 n_channels\n\
int16    n_frequency\n\
int16[] l_channel_data\n\
int16[] r_channel_data\n\
\n\
";

    yarp::os::Type getType() const override
    {
        yarp::os::Type typ = yarp::os::Type::byName(typeName, typeName);
        typ.addProperty("md5sum", yarp::os::Value(typeChecksum));
        typ.addProperty("message_definition", yarp::os::Value(typeText));
        return typ;
    }
};

} // namespace rosmsg
} // namespace yarp

#endif // YARP_ROSMSG_Sound_h
