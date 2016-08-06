#-------------------------------------------------------------------------------
#   Oryol NuklearUI integration module
#-------------------------------------------------------------------------------
fips_begin_module(NKUI)
    fips_vs_warning_level(3)
    fips_files(
        NKUI.h NKUI.cc NKUISetup.h nkuiWrapper.h nkuiWrapper.cc nuklear_config.h
    )
    oryol_shader(NKUIShaders.shd)
    fips_deps(Gfx Input)
fips_end_module()
