# ##############################################################################
# apps/industry/apc3/CMakeLists.txt
# ##############################################################################

if(CONFIG_INDUSTRY_APC3)
  nuttx_add_application(
    NAME
    ${CONFIG_INDUSTRY_APC3_PROGNAME}
    STACKSIZE
    ${CONFIG_INDUSTRY_APC3_STACKSIZE}
    PRIORITY
    ${CONFIG_INDUSTRY_APC3_PRIORITY})
endif()
