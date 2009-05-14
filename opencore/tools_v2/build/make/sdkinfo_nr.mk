# This makefile will generate versioning files based on information from the
# source control system, if it's available.  If it's not, then it will use a
# timestamp
#
# The following variables are specific to the SDK and should be defined locally:
# sdkinfo_target
# sdkinfo_header_filename 
# sdkinfo_header_macro
# sdkinfo_label_macro
# sdkinfo_date_macro
# sdkinfo_version_file
# sdkinfo_version_type

# Skip all the rules if the target is clean
ifneq "$(MAKECMDGOALS)" "clean"

# Check if variables are set, if not set to default
ifeq ($(strip $(GREP)),)
    GREP := grep
endif

ifeq ($(strip $(CUT)),)
    CUT := cut
endif

ifeq ($(strip $(ACCUREV)),)
    ACCUREV := accurev
endif

ifneq ($(strip $(OFFICIAL_RELEASE)),)
sdkinfo_official_string := // *** OFFICIAL RELEASE INFO -- Will not auto update
else
sdkinfo_official_string := 
endif

sdkinfo_official_grep := "*** OFFICIAL RELEASE"


# Initialize values to empty
file_official:=
file_data:=

# Read data from current file (if any)
# First check if file is there
file_exists := $(wildcard $(sdkinfo_header_filename))
ifneq ($(strip $(file_exists)),) 
   file_sdkinfo:= $(shell $(GREP) $(sdkinfo_label_macro) $(sdkinfo_header_filename) | $(CUT) -f2 -d\")
   file_header_time:= $(shell $(GREP) $(sdkinfo_date_macro) $(sdkinfo_header_filename) | $(CUT) -f3 -d" ")

ifeq ($(strip $(OFFICIAL_RELEASE)),)
  # only set file_official and file_data if the OFFICIAL_RELEASE is NOT set.
  # otherwise leave these empty so the file will be regenerated.

# check if file is marked as an official release internally
   file_official:= $(shell $(GREP) $(sdkinfo_official_grep) $(sdkinfo_header_filename))

  # Concat the strings for comparison.  We only want to generate the new file
  # if something has changed
  file_data:=$(join $(file_sdkinfo), $(file_header_time))
endif

endif



ifeq ($(strip $(file_official)),)


ifeq ($(HOST_ARCH),win32)
    DATE := gnu_date
    ECHO := gnu_echo
    header_date_format := 0x%%Y%%m%%d
else
    DATE := date
    ECHO := echo
    header_date_format := 0x%Y%m%d
endif

# Check if date_format strings are good.
# On some PCs, this needs to change - not sure why.
time_okay := $(shell $(DATE) -u +"$(header_date_format)")
ifneq (,$(findstring %, $(time_okay)))
    # swap settings
    ifeq ($(HOST_ARCH),win32)
        header_date_format := 0x%Y%m%d
    else
        header_date_format := 0x%%Y%%m%%d
    endif
endif


# Set prerequisites since depending what is passed in, we may build multiple
# files
sdkinfo_target_prereq := FORCE

header_time:= $(shell $(DATE) -u +"$(header_date_format)")

# Get basis of current accurev workspace
basis_stream := $(strip $(shell $(ACCUREV) info 2>&1 | $(GREP) "Basis:" | $(CUT) -f2 -d":"))

ifneq ($(strip $(basis_stream)),)
    basis_stream_type := $(shell $(ACCUREV) show -fx -s $(basis_stream) streams | $(GREP) "type=" | $(CUT) -f2 -d"\"")
    ifeq ($(strip $(basis_stream_type)),passthrough)
        parent_basis_stream := $(shell $(ACCUREV) show -fx -s $(basis_stream) streams | $(GREP) "basis=" | $(CUT) -f2 -d"\"")
        stream_name := $(parent_basis_stream)
        transaction_id := $(shell $(ACCUREV) hist -ft -t "now".1 -s $(parent_basis_stream) | $(GREP) "transaction" | $(CUT) -f1 -d";" | $(CUT) -f2 -d" " )
    else 
        ifeq ($(strip $(basis_stream_type)),snapshot)
            transaction_id := $(shell $(ACCUREV) show -fx -s $(basis_stream) streams | $(GREP) "name=" | $(CUT) -f2 -d"\"")
        else # basis is normal stream type
            stream_name := $(basis_stream)
            transaction_id := $(shell $(ACCUREV) hist -ft -t "now".1 -s $(basis_stream) | $(GREP) "transaction" | $(CUT) -f1 -d";" | $(CUT) -f2 -d" " )
        endif
    endif
endif

ifeq ($(strip $(transaction_id)),)
    sdkinfo := null
else
    sdkinfo := $(transaction_id)
endif

ifneq ($(strip $(stream_name)),)
   sdkinfo := $(stream_name).$(sdkinfo)
endif

ifneq ($(strip $(sdkinfo_version_file)),)
   sdkinfo_target_prereq := $(sdkinfo_version_file) FORCE
endif

data:=$(join $(sdkinfo), $(header_time))

# $(info header file $(sdkinfo_header_filename), file data = $(file_data), data = $(data))

ifneq ($(strip $(file_data)),$(data))

$(sdkinfo_target): $(sdkinfo_header_filename)

$(sdkinfo_header_filename): sdkinfo_rule_hdr_macro := $(sdkinfo_header_macro)
$(sdkinfo_header_filename): sdkinfo_rule_label_macro := $(sdkinfo_label_macro)
$(sdkinfo_header_filename): sdkinfo_rule_date_macro := $(sdkinfo_date_macro)
$(sdkinfo_header_filename): sdkinfo_rule_sdkinfo := $(sdkinfo)
$(sdkinfo_header_filename): sdkinfo_rule_official_string := $(sdkinfo_official_string)
$(sdkinfo_header_filename): sdkinfo_rule_time := $(header_time)
$(sdkinfo_header_filename): sdkinfo_rule_file_data := $(file_data)
$(sdkinfo_header_filename): sdkinfo_rule_data := $(data)

$(sdkinfo_header_filename): $(sdkinfo_target_prereq)
	@$(ECHO) "Writing $@..."
	@$(ECHO) "#ifndef $(sdkinfo_rule_hdr_macro)_H_INCLUDED" > $@
	@$(ECHO) "#define $(sdkinfo_rule_hdr_macro)_H_INCLUDED" >> $@
	@$(ECHO) "" >> $@
	@$(ECHO) "// This header file is automatically generated at build-time" >> $@
	@$(ECHO) "$(sdkinfo_rule_official_string)" >> $@
	@$(ECHO) "" >> $@
	@$(ECHO) "#define $(sdkinfo_rule_label_macro) \"$(sdkinfo_rule_sdkinfo)\"" >> $@
	@$(ECHO) "#define $(sdkinfo_rule_date_macro) $(sdkinfo_rule_time)" >> $@
	@$(ECHO) "" >> $@
	@$(ECHO) "#endif //$(sdkinfo_rule_hdr_macro)_H_INCLUDED" >> $@
endif

# endif for official release file check
endif




$(sdkinfo_version_file): sdkinfo_rule_sdkinfo := $(sdkinfo)
$(sdkinfo_version_file): sdkinfo_rule_header_time := $(header_time)

ifeq ($(strip $(sdkinfo_version_type)),win32)
$(sdkinfo_version_file): FORCE
	@$(ECHO) "Writing $@..."
	@$(ECHO) "STRINGTABLE DISCARDABLE" > $@
	@$(ECHO) "BEGIN" >> $@
	@$(ECHO) "   IDS_PV_VERSION          \"$(sdkinfo_rule_sdkinfo)\"" >> $@
	@$(ECHO) "END" >> $@
	@$(ECHO) "" >> $@
	@$(ECHO) "STRINGTABLE DISCARDABLE" >> $@
	@$(ECHO) "BEGIN" >> $@
	@$(ECHO) "   IDS_PV_BUILD          \"$(sdkinfo_rule_header_time)\"" >> $@
	@$(ECHO) "END" >> $@
endif

ifeq ($(strip $(sdkinfo_version_type)),winmobile)
$(sdkinfo_version_file): FORCE
	@$(ECHO) "Writing $@..."
	@$(ECHO) "STRINGTABLE" > $@
	@$(ECHO) "BEGIN" >> $@
	@$(ECHO) "   IDS_PV_VERSION          \"$(sdkinfo_rule_sdkinfo)\"" >> $@
	@$(ECHO) "END" >> $@
	@$(ECHO) "" >> $@
endif


ifeq ($(strip $(sdkinfo_version_type)),s60v3)
$(sdkinfo_version_file): FORCE
	@$(ECHO) "Writing $@..."
	@$(ECHO) "_LIT(KPVVersionNumber, \"$(sdkinfo_rule_sdkinfo)\");" > $@
	@$(ECHO) "_LIT(KPVBuildNumber, \"$(sdkinfo_rule_header_time)\");" >> $@
	@$(ECHO) "" >> $@
endif


# endif for this not being for the clean target
endif 
