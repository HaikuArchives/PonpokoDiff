# PonpokoDiff
#
# Copyright (c) 2008-2009 PonpokoDiff Project Contributors
# 
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
# 
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

ifeq ($(MAKECMDGOALS), release)
RELEASE = true
NODEPENDS = true
endif

ifeq ($(MAKECMDGOALS), clean)
NODEPENDS = true
endif

# ------
NAME = PonpokoDiff

CPPFLAGS = -Wall -Wno-multichar -Wno-ctor-dtor-privacy -Woverloaded-virtual
LDFLAGS = -Xlinker -soname=_APP_

ifneq ($(RELEASE), true)
DEBUG = true
endif

ifeq ($(DEBUG), true)
OBJ_DIR = obj.debug
CPPFLAGS += -O0 -g -DDEBUG
LDFLAGS += -g
else
OBJ_DIR = obj.release
CPPFLAGS += -O1
LDFLAGS += -Xlinker -s -Xlinker -x
endif

#%{
# @src->@

SRCS = \
	Exception.cpp \
	LineSeparatedText.cpp \
	NPDiff.cpp \
	OpenFilesDialog.cpp \
	PonpokoDiffApp.cpp \
	ResourceText.cpp \
	Substring.cpp \
	TextDiffView.cpp \
	TextDiffWnd.cpp \


RDEFS = \
	Strings.rdef \


RSRCS = \


ifeq ($(TARGETTYPE), be)
	RDEFS += PonpokoDiffBe.rdef
else
	RDEFS += PonpokoDiffHaiku.rdef
endif

# @<-src@
#%}

LIBS = stdc++.r4 be root tracker

LIB_PATHS =

SYSTEM_INCLUDE_PATHS = 

LOCAL_INCLUDE_PATHS = 

DEFINES = 





# ------

CC = gcc
LD = gcc
MIMESET = mimeset
RC = rc
XRES = xres

TARGET = $(OBJ_DIR)/$(NAME)

define SRCS_LIST_TO_OBJS
	$(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(foreach file, $(SRCS), \
	$(basename $(notdir $(file))))))
endef

OBJS = $(SRCS_LIST_TO_OBJS)
DEPENDS = $(patsubst %.o,%.d,$(OBJS))

define RDEF_LIST_TO_RDEFRSRCS
	$(addprefix $(OBJ_DIR)/, $(addsuffix .rsrc, $(foreach file, $(RDEFS), \
	$(basename $(notdir $(file))))))
endef

RSRCS += $(RDEF_LIST_TO_RDEFRSRCS)

SRC_PATHS += $(sort $(foreach file, $(SRCS), $(dir $(file))))

VPATH :=
VPATH += $(addprefix :, $(subst  ,:, $(filter-out $($(subst, :, ,$(VPATH))), $(SRC_PATHS))))

LOC_INCLUDES = $(foreach path, $(SRC_PATHS) $(LOCAL_INCLUDE_PATHS), $(addprefix -I, $(path)))
SYS_INCLUDES += -I-
SYS_INCLUDES += $(foreach path, $(SYSTEM_INCLUDE_PATHS), $(addprefix -I, $(path)))

INCLUDES = $(LOC_INCLUDES) $(SYS_INCLUDES)

LINK_PATHS = $(foreach path, $(SRC_PATHS) $(LIBPATHS), $(addprefix -L, $(path)))

LINK_LIBS += $(foreach lib, $(LIBS), $(addprefix -l, $(lib)))

LDFLAGS += $(LINK_PATHS) $(LINK_LIBS)

CPPFLAGS += $(foreach define, $(DEFINES), $(addprefix -D, $(define)))

BUILD_LINE = $(LD) -o $@ $(OBJS) $(LDFLAGS)

ifeq ($(RSRCS), )
	DO_RSRCS :=
else
	DO_RSRCS := $(XRES) -o "$(TARGET)" $(RSRCS)
endif

# ------ targets

.PHONY: all
all : $(TARGET)

$(TARGET) : $(OBJ_DIR) $(OBJS) $(RSRCS)
	$(BUILD_LINE)
	$(DO_RSRCS)
	$(MIMESET) -f $@

$(OBJ_DIR)::
	@[ -d $(OBJ_DIR) ] || mkdir $(OBJ_DIR) > /dev/null 2>&1

$(OBJ_DIR)/%.o : %.cpp
	$(CC) -c $< $(INCLUDES) $(CPPFLAGS) -o $@

$(OBJ_DIR)/%.rsrc : %.rdef
	rc -o $@ $<

$(OBJ_DIR)/%.d: %.cpp
	@echo updating dependencies for $(notdir $<)
	@$(CC) -MM $< $(INCLUDES) $(CPPFLAGS) \
	| sed 's/\($*\)\.o[ :]*/$(OBJ_DIR)\/\1.o $(OBJ_DIR)\/\1.d : /g' > $@; \
	[ -s $@ ] || rm -f $@

ifneq ($(NODEPENDS), true)
-include $(DEPENDS)
endif

.PHONY: release
release : clean all


.PHONY: clean
clean :
	-rm -rf $(OBJ_DIR)/*
