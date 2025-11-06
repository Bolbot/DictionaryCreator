use std::collections::HashSet;

pub trait RegularExpressionParser {
    fn single_match(&self, source: &str, offset: i32) -> String;
    fn all_matches(&self, source: &str, offset: i32) -> HashSet<String>;   // TODO: multiset
}

pub mod pcre_parser {
    use crate::regex_parser::RegularExpressionParser;

    pub struct RegexParser {
    }
    impl RegexParser {
        pub fn new(pattern: &str) -> RegexParser {
            // TODO
            RegexParser {  }
        }
    }
    impl RegularExpressionParser for RegexParser {
        fn single_match(&self, source: &str, offset: i32) -> String {
            unimplemented!("under construction");
        }
        fn all_matches(&self, source: &str, offset: i32) -> std::collections::HashSet<String> {
            unimplemented!("under construction");
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::regex_parser::RegularExpressionParser;

    use super::pcre_parser::RegexParser;

    #[test]
    #[should_panic]
    fn construction_in_progress_for_single_match() {
        let parser = RegexParser::new("panics");
        parser.single_match(String::new().as_str(), 0);
    }

    #[test]
    #[should_panic]
    fn construction_in_progress_for_all_matches() {
        let parser = RegexParser::new("panics");
        parser.all_matches(String::new().as_str(), 0);
    }
}
