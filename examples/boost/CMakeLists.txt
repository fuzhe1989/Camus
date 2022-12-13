target_add_example(boost_example main.cpp)
target_link_libraries (boost_example Boost::boost)
print_target_properties(boost_example)
