
$(strip $(foreach mk,$(strip $(LOCAL_MAKEFILES_LIST)),$(eval include $(mk))))

