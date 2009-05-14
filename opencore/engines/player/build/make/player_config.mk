# The conditions in this makefile fragment would be removed once
# the projects are configured using CML2.
# It is not allowed to add any more conditions in this file.

# Add the include path based on project to pick the right tunable and config header
#
ifeq ($(ARCHITECTURE),android)
    XINCDIRS += -I ../../config/$(BUILD_ARCH)
else
    ifeq ($(FORMAT),3gpp)
        XINCDIRS += -I ../../config/linux_3gpp
    else
        ifeq ($(ARCHITECTURE),panasonic_arm)
            XINCDIRS += -I ../../config/$(BUILD_ARCH)
         else
            ifeq ($(ARCHITECTURE),panasonic_pc)
                XINCDIRS += -I ../../config/$(BUILD_ARCH)
            else
                ifeq ($(ARCHITECTURE),nec_hsdpa_linux)
                    XINCDIRS += -I ../../config/$(BUILD_ARCH)
                else
                    ifeq ($(ARCHITECTURE),nec_hsdpa_arm)
                        XINCDIRS += -I ../../config/$(BUILD_ARCH)
                    else
                        ifeq ($(FORMAT),real)
                            XINCDIRS += -I ../../config/linux_real
                        else
                            ifeq ($(FORMAT),asf)
                                XINCDIRS += -I ../../config/linux_asf
                            else
                                ifeq ($(ARCHITECTURE),necel_arm)
                                    XINCDIRS += -I ../../config/$(BUILD_ARCH)
                                else
                                    ifeq ($(FORMAT),android)
                                        XINCDIRS += -I ../../config/android
                                    else
                                        ifeq ($(ARCHITECTURE),farnsworth_arm)
                                            XINCDIRS += -I ../../config/$(BUILD_ARCH)
                                        else
                                            ifeq ($(FORMAT),linux_non_omx)
                                                XINCDIRS += -I ../../config/linux_non_omx
                                            else
                                                XINCDIRS += -I ../../config/linux
                                            endif
                                        endif
                                    endif
                                endif
                            endif
                        endif
                    endif
                endif
            endif
         endif
    endif
endif

# Pick the node registry based on project
#
ifeq ($(USE_CML2_CONFIG), 1)
    SRCS += ../config/core/pv_player_node_registry_populator.cpp
else
    ifeq ($(ARCHITECTURE),panasonic_arm)
        SRCS += ../config/$(BUILD_ARCH)/pv_player_node_registry_populator.cpp
    else
        ifeq ($(ARCHITECTURE),panasonic_pc)
    	    SRCS += ../config/$(BUILD_ARCH)/pv_player_node_registry_populator.cpp
		else
            ifeq ($(ARCHITECTURE),nec_hsdpa_linux)
	        	SRCS += ../config/$(BUILD_ARCH)/pv_player_node_registry_populator.cpp
	        else
    	        ifeq ($(ARCHITECTURE),nec_hsdpa_arm)
        	        SRCS += ../config/$(BUILD_ARCH)/pv_player_node_registry_populator.cpp
            	else
	                ifeq ($(FORMAT),real)
    	                SRCS += ../config/linux_real/pv_player_node_registry_populator.cpp
        	        else
            	        ifeq ($(FORMAT),asf)
                	        SRCS += ../config/linux_asf/pv_player_node_registry_populator.cpp
                    	else
	                        ifeq ($(ARCHITECTURE),necel_arm)
    	                        SRCS += ../config/$(BUILD_ARCH)/pv_player_node_registry_populator.cpp
        	                else
            	                ifeq ($(FORMAT),android)
                	                SRCS += ../config/android/pv_player_node_registry_populator.cpp
                    	        else
                        	        ifeq ($(ARCHITECTURE),farnsworth_arm)
                            	        SRCS += ../config/$(BUILD_ARCH)/pv_player_node_registry_populator.cpp
                                	else
	                                    ifeq ($(FORMAT),linux_non_omx)
    	                                    SRCS += ../config/linux_non_omx/pv_player_node_registry_populator.cpp
        	                            else
            	                            ifeq ($(FORMAT),3gpp)
                	                            SRCS += ../config/linux_3gpp/pv_player_node_registry_populator.cpp
                    	                    else
                        	                    SRCS += ../config/linux/pv_player_node_registry_populator.cpp
                            	            endif
                                	    endif
	                                endif
    	                        endif 
        	                endif
            	        endif
                	endif
	            endif
    	    endif
	    endif
	endif
endif
