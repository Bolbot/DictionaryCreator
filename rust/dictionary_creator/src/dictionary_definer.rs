use super::dictionary_types::definitions_t;
use super::dictionary_language::Language;

pub fn percent_encode(input: String) -> String {
    unimplemented!("under construction");
}

pub fn define_word(word: String, language: Language) -> definitions_t {
    unimplemented!("under construction");
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    #[should_panic]
    fn construction_in_progress_for_percent_encode() {
        assert_eq!(String::new(), percent_encode(String::new()));
    }

    #[test]
    #[should_panic]
    fn construction_in_progress_for_define_word() {
        assert_eq!(definitions_t::new(), define_word(String::new(), Language::English));
    }

    #[test]
    #[should_panic]
    fn connections_is_visible() {
        auxiliary::connections::get("panics");
    }

    #[test]
    #[should_panic]
    fn json_parser_is_visible() {
        let whatever = auxiliary::json_parser::parse_json_to_definitions_set("panics");
    }
}
