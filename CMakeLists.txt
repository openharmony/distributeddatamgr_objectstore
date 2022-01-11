cmake_minimum_required(VERSION 3.16)
project(hanlu)


set(CMAKE_CXX_STANDARD 17)

include_directories(./base/include)
include_directories(./interfaces/innerkits)
include_directories(./frameworks/jskitsimpl/include/adaptor)
include_directories(./frameworks/jskitsimpl/include/common)
include_directories(./frameworks/innerkitsimpl/include/adaptor)
include_directories(./frameworks/innerkitsimpl/include/common)
include_directories(./frameworks/innerkitsimpl/include/communicator)
add_library(hanlu
        frameworks/jskitsimpl/src/adaptor/js_distributedobjectstore.cpp
        frameworks/innerkitsimpl/src/adaptor/distributed_object_store_impl.cpp
        frameworks/innerkitsimpl/src/adaptor/distributed_object_impl.cpp
        frameworks/innerkitsimpl/src/adaptor/flat_object_storage_engine.cpp
        frameworks/innerkitsimpl/src/adaptor/flat_object_store.cpp
        frameworks/innerkitsimpl/src/communicator/app_device_handler.cpp
        frameworks/innerkitsimpl/src/communicator/app_pipe_handler.cpp
        frameworks/innerkitsimpl/src/communicator/app_pipe_mgr.cpp
        frameworks/innerkitsimpl/src/communicator/ark_communication_provider.cpp
        frameworks/innerkitsimpl/src/communicator/communication_provider.cpp
        frameworks/innerkitsimpl/src/communicator/communication_provider_impl.cpp
        frameworks/innerkitsimpl/src/communicator/process_communicator_impl.cpp
        frameworks/innerkitsimpl/src/communicator/softbus_adapter_standard.cpp
        frameworks/jskitsimpl/src/adaptor/js_watcher.cpp
        frameworks/jskitsimpl/src/adaptor/js_distributedobject.cpp
        frameworks/jskitsimpl/src/adaptor/js_object_wrapper.cpp
        frameworks/jskitsimpl/src/adaptor/js_module_init.cpp
        frameworks/jskitsimpl/src/common/js_util.cpp
        )
