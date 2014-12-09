
event_table = {
   shutdown       = "e_exit",
   Calibrate = "e_calibrate",
   Where     = "e_where",
   I         = "e_i_will",
   Take      = "e_take",
   Return    = "e_home",
   Touch     = "e_touch",
   Push      = "e_push",
   Forget    = "e_forget",
   Explore   = "e_explore",
   What      = "e_what",
   Let       = "e_let",
   look     = "e_look",
   point    = "e_point",
   }

interact_fsm = rfsm.state{

    ----------------------------------
    -- state SUB_MENU               --
    ----------------------------------
    SUB_MENU = rfsm.state{
        entry=function()
            print("in substate MENU : waiting for speech command!")
        end,

        doo = function()
            while true do
                -- speak(ispeak_port, "What should I do?")
                result = SM_Reco_Grammar(speechRecog_port, grammar)
                print("received REPLY: ", result:toString() )
                local cmd = result:get(3):asString()
                rfsm.send_events(fsm, event_table[cmd])
                rfsm.yield(true)
            end
        end
    },

    ----------------------------------
    -- states                       --
    ----------------------------------

    SUB_EXIT = rfsm.state{
        entry=function()
            speak(ispeak_port, "Ok, bye bye")
            rfsm.send_events(fsm, 'e_menu_done')
        end
    },

    SUB_CALIBRATE = rfsm.state{
        entry=function()
            speak(ispeak_port,"Asked to calibrate")
        end
    },

    SUB_WHERE = rfsm.state{
        entry=function()
            local obj = result:get(7):asString()
            speak(ispeak_port,"Asked to find the")
            speak(ispeak_port, obj)
        end
    },

    SUB_TEACH_OBJ = rfsm.state{
        entry=function()
            speak(ispeak_port,"Asked to teach a new object")
        end
    },

    SUB_TAKE = rfsm.state{
        entry=function()
            local obj = result:get(5):asString()
            speak(ispeak_port,"Asked to take the")
            speak(ispeak_port, obj)
        end
    },
    
    SUB_RETURN = rfsm.state{
        entry=function()
            speak(ispeak_port,"Asked to return to home position")
        end
    },
    
    SUB_TOUCH = rfsm.state{
        entry=function()
            local obj = result:get(5):asString()
            speak(ispeak_port,"Asked to touch the")
            speak(ispeak_port, obj)
        end
   },

    SUB_PUSH = rfsm.state{
        entry=function()
            local obj = result:get(5):asString()
            speak(ispeak_port,"Asked to push the")
            speak(ispeak_port, obj)
        end
    },

    SUB_FORGET = rfsm.state{
        entry=function()
            local obj = result:get(3):asString()
            speak(ispeak_port,"Asked to forget the")
            speak(ispeak_port, obj)
        end
    },

    SUB_EXPLORE = rfsm.state{
        entry=function()
            local obj = result:get(5):asString()
            speak(ispeak_port,"Asked to explore the")
            speak(ispeak_port, obj)
        end
    },
    
    SUB_WHAT = rfsm.state{ 
        entry=function()
        speak(ispeak_port,"Asked to find what is this")
        end
    },

    SUB_LET = rfsm.state{
        entry=function()
            speak(ispeak_port,"Asked to let me teach")
        end
    };

    SUB_KIN = rfsm.state{
            entry=function()
            speak(ispeak_port,"Asked to ")
        end
    };
    
    SUB_LOOK = rfsm.state{
        entry=function()
            speak(ispeak_port,"I see you there")
            result = command_function(speechCommand_port,"look")
            print("received REPLY: ", result:toString() )
        end
    };   
     
    SUB_POINT = rfsm.state{
        entry=function()
            speak(ispeak_port,"You are there")
            result = command_function(speechCommand_port,"poin")
            print("received REPLY: ", "OK" )
        end  
    };
   ----------------------------------
   -- state transitions            --
   ----------------------------------

   rfsm.trans{ src='initial', tgt='SUB_MENU'},
   rfsm.transition { src='SUB_MENU', tgt='SUB_EXIT', events={ 'e_exit' } },

   rfsm.transition { src='SUB_MENU', tgt='SUB_CALIBRATE', events={ 'e_calibrate' } },
   rfsm.transition { src='SUB_CALIBRATE', tgt='SUB_MENU', events={ 'e_done' } },

   rfsm.transition { src='SUB_MENU', tgt='SUB_WHERE', events={ 'e_where' } },
   rfsm.transition { src='SUB_WHERE', tgt='SUB_MENU', events={ 'e_done' } },

   rfsm.transition { src='SUB_MENU', tgt='SUB_TEACH_OBJ', events={ 'e_i_will' } },
   rfsm.transition { src='SUB_TEACH_OBJ', tgt='SUB_MENU', events={ 'e_done' } },

   rfsm.transition { src='SUB_MENU', tgt='SUB_TAKE', events={ 'e_take' } },
   rfsm.transition { src='SUB_TAKE', tgt='SUB_MENU', events={ 'e_done' } },

   rfsm.transition { src='SUB_MENU', tgt='SUB_RETURN', events={ 'e_home' } },
   rfsm.transition { src='SUB_RETURN', tgt='SUB_MENU', events={ 'e_done' } },

   rfsm.transition { src='SUB_MENU', tgt='SUB_TOUCH', events={ 'e_touch' } },
   rfsm.transition { src='SUB_TOUCH', tgt='SUB_MENU', events={ 'e_done' } },

   rfsm.transition { src='SUB_MENU', tgt='SUB_PUSH', events={ 'e_push' } },
   rfsm.transition { src='SUB_PUSH', tgt='SUB_MENU', events={ 'e_done' } },

   rfsm.transition { src='SUB_MENU', tgt='SUB_FORGET', events={ 'e_forget' } },
   rfsm.transition { src='SUB_FORGET', tgt='SUB_MENU', events={ 'e_done' } },

   rfsm.transition { src='SUB_MENU', tgt='SUB_EXPLORE', events={ 'e_explore' } },
   rfsm.transition { src='SUB_EXPLORE', tgt='SUB_MENU', events={ 'e_done' } },

   rfsm.transition { src='SUB_MENU', tgt='SUB_WHAT', events={ 'e_what' } },
   rfsm.transition { src='SUB_WHAT', tgt='SUB_MENU', events={ 'e_done' } },

   rfsm.transition { src='SUB_MENU', tgt='SUB_LET', events={ 'e_let' } },
   rfsm.transition { src='SUB_LET', tgt='SUB_MENU', events={ 'e_done' } },

   rfsm.transition { src='SUB_LET', tgt='SUB_KIN', events={ 'e_kin' } },
   rfsm.transition { src='SUB_KIN', tgt='SUB_MENU', events={ 'e_done' } },
   
   rfsm.transition { src='SUB_MENU', tgt='SUB_LOOK', events={ 'e_look'}},
   rfsm.transition { src='SUB_LOOK', tgt="SUB_MENU", events={ 'e_done'}},

   rfsm.transition { src='SUB_MENU', tgt='SUB_POINT', events={ 'e_point'}},
   rfsm.transition { src='SUB_POINT', tgt="SUB_MENU", events={ 'e_done'}},
}
