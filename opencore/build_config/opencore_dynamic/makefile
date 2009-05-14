
include $(MK)/entry_point.mk

FORMAT := android

include $(CFG_DIR)/../common/local.mk

TESTAPPS="pvplayer_engine_test test_pvauthorengine pv2way_omx_engine_test"
TESTAPP_DIR_pvplayer_engine_test="/engines/player/test/build/android"
TESTAPP_DIR_test_pvauthorengine="/engines/author/test/build/android"
TESTAPP_DIR_pv2way_omx_engine_test="/engines/2way/test/build/make"

opencore_common_PRELINK := true
opencore_player_PRELINK := true
opencore_author_PRELINK := true
opencore_2way_PRELINK := true

include $(MK)/cml2_shared.mk

