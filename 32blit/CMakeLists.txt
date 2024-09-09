find_package (32BLIT CONFIG REQUIRED PATHS ../32blit-sdk)

blit_executable(thumb-test
    main.cpp
    ../tests/tests.c
    ../tests/load_store_tests.c
)
blit_metadata(thumb-test metadata.yml)

target_include_directories(thumb-test PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../)

