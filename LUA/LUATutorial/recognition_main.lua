#!/usr/bin/lua

require("rfsm")
require("yarp")

yarp.Network_init()

-------
shouldExit = false

-- initialization
ispeak_port = yarp.BufferedPortBottle()
speechRecog_port = yarp.Port()

-- defining objects and actions vocabularies
objects = {"octopus", "lego", "toy", "ladybug", "turtle", "car", "bottle", "box"}

-- defining speech grammar for Menu

grammar = "Return to home position | Calibrate on table | Where is the #Object | Take the #Object | See you soon  | I will teach you a new object | "
       .."Touch the #Object | Push the #Object | Let me show you how to reach the #Object with your right arm | Let me show you how to reach the #Object with your left arm | "
        .."Forget #Object | Forget all objects | What is this | Explore the #Object "

-- load state machine model and initalize it
rf = yarp.ResourceFinder()
rf:setVerbose()
rf:setDefaultContext("recognition/LUA")
rf:configure(arg)
fsm_file = rf:findFile("recognition_root_fsm.lua")
fsm_model = rfsm.load(fsm_file)
fsm = rfsm.init(fsm_model)

repeat
    rfsm.run(fsm)
    yarp.Time_delay(0.1)
until shouldExit ~= false

print("finishing")
-- Deinitialize yarp network
yarp.Network_fini()
