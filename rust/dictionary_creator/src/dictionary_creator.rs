use super::dictionary_language::Language;
use auxiliary::regex_parser::pcre_parser::RegexParser;

pub struct DictionaryCreator {
    language: Language,
    // TODO: file streams,
    minimal_substantial_word_length: usize,
    terminating_characters: String,
    proper_nouns_extractor: RegexParser,
    linestarting_name_extractor: RegexParser,
    words_extractor: RegexParser
}

impl DictionaryCreator {
    pub fn new(language: Language) -> DictionaryCreator {
        DictionaryCreator {
            language,
            // TODO: language-specific settings, RegEx patterns
            minimal_substantial_word_length: 3,
            terminating_characters: String::new(),
            proper_nouns_extractor: RegexParser::new(""),
            linestarting_name_extractor: RegexParser::new(""),
            words_extractor: RegexParser::new("")
        }
    }
    // TODO: add_input as input file stream
    pub fn parse_to_dictionary(&mut self) {
        unimplemented!("under construction");
    }
    pub fn parse_line(&self, line: String) {
        unimplemented!("under construction");
    }

    // TODO: private function to parse_one_file
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    #[should_panic]
    fn construction_in_progress_for_parse_to_dictionary() {
        let mut creator = DictionaryCreator::new(Language::English);
        creator.parse_to_dictionary();
    }

    #[test]
    #[should_panic]
    fn construction_in_progress_for_parse_line() {
        let creator = DictionaryCreator::new(Language::English);
        creator.parse_line("unimplemented".to_string());
    }
}
