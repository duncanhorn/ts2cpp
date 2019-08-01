
project(ts2cpp)
add_executable(ts2cpp)

target_sources(ts2cpp PRIVATE
    lexer.cpp
    main.cpp
    parser.cpp)
