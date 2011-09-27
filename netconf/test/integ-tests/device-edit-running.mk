# ----------------------------------------------------------------------------|
# Device Edit tests
EDIT_TEST_SUITE_SOURCES := $(YUMA_TEST_SUITE_COMMON)/device-edit-tests.cpp \
                           device-edit-running.cpp \

ALL_SOURCES += $(EDIT_TEST_SUITE_SOURCES) 

ALL_EDIT_TEST_SUITE_SOURCES := $(BASE_SOURCES) $(EDIT_TEST_SUITE_SOURCES)						

test-device-edit-running: $(call ALL_OBJECTS,$(ALL_EDIT_TEST_SUITE_SOURCES)) | yuma-op
	$(MAKE_TEST)

TARGETS += test-device-edit-running
