#[cxx::bridge]
mod ffi {

    struct HypeOutput {
        output: String,
        new_len: i32,
    }

    extern "Rust" {
        fn hypeify(input: String, num_exclamations: i32) -> HypeOutput;
    }

}