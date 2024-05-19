# Set usual component variables
set(COMPONENT_SRCS "rot_btn_i2c_riscv.c")
set(COMPONENT_ADD_INCLUDEDIRS "include")
set(COMPONENT_REQUIRES ulp driver)
register_component()

set(ulp_app_name "ulp_main")
# set(ulp_app_name "ulp_rot_btn_i2c")
set(ulp_riscv_sources "ulp/riscv_main.c")
set(ulp_exp_dep_srcs "rot_btn_i2c_riscv.c")
ulp_embed_binary(${ulp_app_name} "${ulp_riscv_sources}" "${ulp_exp_dep_srcs}")
