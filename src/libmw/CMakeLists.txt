add_library(libmw SHARED "libmw.cpp" "BlockStoreWrapper.h" "Transformers.h" "State.h" "libmw_node.cpp" "libmw_wallet.cpp" "defs.cpp")

#add_dependencies(libmw litecoin MW::Common MW::Crypto MW::Database MW::File MW::MMR MW::Traits MW::Node)
target_link_libraries(libmw PRIVATE litecoin MW::Common MW::Crypto MW::Database MW::File MW::MMR MW::Traits MW::Node MW::Wallet)
set_target_properties(libmw PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN 1
    PREFIX "")
    
# Install library
install(
    TARGETS libmw
    LIBRARY DESTINATION lib
)