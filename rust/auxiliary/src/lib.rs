pub mod connections;

pub mod json_parser;

pub mod regex_parser;
use regex_parser::pcre_parser::RegexParser;

pub mod fs_manager;

#[cfg(test)]
mod tests {
    use std::collections::{ HashMap, HashSet };

    use crate::regex_parser::RegularExpressionParser;

    use super::*;

    #[test]
    #[should_panic]
    fn temp_visibility_connections() {
        assert_eq!(connections::get("unimplemented!"), "panics");
    }

    #[test]
    #[should_panic]
    fn temp_visibility_json_parser() {
        assert_eq!(json_parser::parse_json_to_definitions_set("unimplemented"), HashSet::new());
        assert_eq!(json_parser::parse_json_to_definitions_map("unimplemented"), HashMap::new());
    }

    #[test]
    #[should_panic]
    fn temp_visibility_regex_parser() {
        let parser = RegexParser::new("just empty");
        parser.single_match(String::from("panics").as_str(), 0);
    }

    #[test]
    #[should_panic]
    fn temp_visibility_fs_manager() {
        assert_eq!(fs_manager::get_input_file_path(), "panics");
        assert_eq!(fs_manager::get_output_file_path(), "panics");
    }
}
