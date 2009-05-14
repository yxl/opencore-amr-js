# This makefile will generate documents from a set of header files

# The following variables are specific to the SDK and should be defined locally:
# doc_target
# doc_title
# doc_paths
# doc_version

doc_$(doc_target): doc_rule_target := $(doc_target)
doc_$(doc_target): doc_rule_title := $(doc_title)
doc_$(doc_target): doc_rule_paths := $(doc_paths)
doc_$(doc_target): doc_rule_version := $(doc_version)

doc_$(doc_target):
	perl $(SRC_ROOT)/tools_v2/build/document/bin/doc_build.bat --doctype $(doc_rule_target) --title $(doc_rule_title) --path $(doc_rule_paths) --filetype "mainpage *.h readme.txt" --exclude_pattern "*/test/* */obsolete/* */doxydir/*" -ver $(doc_rule_version)
