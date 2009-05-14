# Function that returns the path information of the makefile.
# For this to work reliably, it must be called within the local
# makefile before any other include statements.
define get_makefile_dir
   $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
endef

# The definitions below give the basic definition of a space 
# which is useful in subst calls where it is desirable to remove spaces.
SPACE:=
SPACE+=

# template to clean up strings by removing quotes
define remove_quotes
$(strip $(subst ",,$1))
endef

# Simple function to clear a variable
define clearvar
endef

# Simple function to process a string containing a list of makefile fragments
# to include
define process_include_list
$(patsubst %,$1/%,$(call remove_quotes,$2))
endef

# Simple function to process a string containing a list of 
# preprocessor macros
define process_config_macros
$(patsubst %,-D%,$(call remove_quotes,$1))
endef



# Function returns a list with one fewer elements.  
# The last element is dropped.
truncate = $(wordlist 2,$(words $1),x $1)


# Function removes duplicate entries in a list without changing the order
purge_duplicates = $(strip $(if $1,$(call purge_duplicates,$(call truncate,$1)) $(if $(filter $(lastword $1),$(call truncate,$1)),,$(lastword $1))))

ifeq ($(strip $(RELEASE)),1)
  $(info Building in release mode)
   DEFAULT_LIBMODE := release
else
   DEFAULT_LIBMODE := debug
endif


# Function to remove two levels of directories
define strip_two_levels_up
 $(foreach path,$(1),$(patsubst %,/%,$(subst $(SPACE),/,$(call truncate,$(call truncate,$(subst /, ,$(dir $(path)))))))) 
endef

# Function to remove ../../
define go_up_two_levels
  $(patsubst ../../%,%,$1)
endef

# Utility functions
[A-Z] := A B C D E F G H I J K L M N O P Q R S T U V W X Y Z #
[a-z] := a b c d e f g h i j k l m n o p q r s t u v w x y z #

# replace delimiter with space, so that it can be split
# split(delimiter, string)
split = $(strip $(subst $1, ,$2))

# translate character list in traslate-from to corresponding translate-to in string/text
# translate(translate-from, translate-to, string)
translate = $(strip $(eval tmp := $3) $(foreach c, $(join $(addsuffix :,$1),$2),       \
     $(eval tmp := $(subst $(word 1,$(subst :, ,$c)),$(word 2,$(subst :, ,$c)), \
     $(tmp))))$(tmp))

# converts string into uppercase
# uppercase(string)
uppercase = $(call translate,$([a-z]),$([A-Z]),$1)

# converts string into lowercase
# lowercase(string)
lowercase = $(call translate,$([A-Z]),$([a-z]),$1)

