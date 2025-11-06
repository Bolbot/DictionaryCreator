pub mod dictionary_types;

pub mod dictionary_entry;

pub mod dictionary;

pub mod dictionary_language;

pub mod dictionary_exporter;

pub mod dictionary_creator;

pub mod dictionary_definer;

pub mod dictionary_manager;

#[cfg(test)]
mod tests {
    use std::collections::{ HashMap, HashSet };

    use super::*;

    fn mock_definer(whatever: &str) -> dictionary_types::definitions_t {
        let mut result = HashMap::new();
        result.insert(whatever.to_string(), HashSet::new());
        result
    }

    #[test]
    fn dictionary_types_are_visible() {
        let empty = dictionary_types::definitions_t::new();
        assert_eq!(empty, HashMap::new());

        let local_definer : dictionary_types::definer_t = mock_definer;
        assert_eq!(local_definer("whatever").len(), 1);
    }

    #[test]
    fn dictionary_entry_is_visible() {
        let entry = dictionary_entry::Entry::new("future".to_string());
        assert!(!entry.is_defined());
    }

    #[test]
    fn dictionary_is_visible() {
        let dictionary = dictionary::Dictionary::new(dictionary_language::Language::English);
        assert_eq!(dictionary.total_words(), 0);
    }

    #[test]
    fn dictionary_exporter_is_visible() {
        let exporter = dictionary_exporter::DictionaryExporter::new();
        assert!(exporter.temporary_visibility_test());
    }

    #[test]
    #[should_panic]
    fn dictionary_definer_is_visible() {
        assert_eq!(dictionary_definer::define_word(dictionary_definer::percent_encode(String::new()),
            dictionary_language::Language::English), HashMap::new());
    }
}
