### Compilation failed:

error: 5: identifiers with multiple underscores in a row are not permitted
int __two_underscores_at_the_front;
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 6: identifiers with multiple underscores in a row are not permitted
int two_underscores__in_the_middle;
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 7: identifiers with multiple underscores in a row are not permitted
int two_underscores_at_the_end__;
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 8: identifiers with multiple underscores in a row are not permitted
int __; // two underscores in isolation
    ^^
error: 10: identifiers with multiple underscores in a row are not permitted
int three___underscores;
    ^^^^^^^^^^^^^^^^^^^
error: 12: identifiers with multiple underscores in a row are not permitted
struct two_underscores__in_a_typename {
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 13: identifiers with multiple underscores in a row are not permitted
    int two_underscores__in_a_struct_variable;
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
7 errors
