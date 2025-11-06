use crate::dictionary_definer::define_word;
use crate::dictionary_entry::Entry;
use crate::dictionary_language::Language;
use super::dictionary::Dictionary;
use super::dictionary_creator::DictionaryCreator;
use super::dictionary_exporter::DictionaryExporter;
use super::dictionary_types::definer_t;

use std::rc::Rc;

struct DictionaryFilename<'a> {
    full: String,
    human_readable: &'a str
}

pub struct DictionaryManager {
    name: String,
    //definer: definer_t,               // TODO: find a way to pass a capturing lambda or create yet another struct
    dictionary: Dictionary,
    creator: DictionaryCreator,
    //exporter: DictionaryExporter      // TODO: streaming operations
}
impl DictionaryManager {
    pub fn new(language: Language, name: String) -> DictionaryManager {
        DictionaryManager {
            name: if name.len() > 0 { name } else { "Default dictionary".to_string() },
            // TODO: find a way to pass a capturing lambda or create yet another struct
            //definer: |word: String| { define_word(word, language) },
            dictionary: Dictionary::new(language),
            creator: DictionaryCreator::new(language),
            // TODO: make it export to stdout
            //exporter: ()
        }
    }

    // TODO: file operations

    pub fn parse_all_pending(&mut self) {
        unimplemented!("under construction");
    }
    pub fn parse_one_line(&mut self, line: String) {
        unimplemented!("under construction");
    }

    // TODO: lookup_or_add_word with arbitrary auxiliary arguments
    pub fn lookup_or_add_word(&mut self, word: String) -> Rc<Entry> {
        unimplemented!("under construction");
    }

    pub fn contains_word(&self, word: String) -> bool {
        unimplemented!("under construction");
    }

    // TODO: wrap subset operations of Dictionary

    pub fn define(&mut self, word: String) -> Rc<Entry> {
        unimplemented!("under construction");
    }
    // TODO: find a way to overload for Rc, subsets, and Entry&

    // TODO: file operations and export

    pub fn rename(&mut self, new_name: String) {
        self.name = new_name;
    }
    pub fn get_name(&self) -> String {
        self.name.clone()
    }
}

// Serialization: farther future plans; starts with save_dictionary in DictionaryManager
fn load_dictionary(file_name: String) -> DictionaryManager {
    unimplemented!("under construction");
}
// TODO: available_dictionaries as vector of previously saved dictionaries


#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    #[should_panic]
    fn construction_in_progress_for_parse_all_pending() {
        let mut manager = DictionaryManager::new(Language::English, "Unimplemented".to_string());
        manager.parse_all_pending();
    }

    #[test]
    #[should_panic]
    fn construction_in_progress_for_lookup_or_add_word() {
        let mut manager = DictionaryManager::new(Language::English, "Unimplemented".to_string());
        assert_eq!(manager.lookup_or_add_word(String::new()).as_str(), Rc::new(Entry::new(String::new())).as_str());
    }

    #[test]
    #[should_panic]
    fn construction_in_progress_for_contains_word() {
        let manager = DictionaryManager::new(Language::English, "Unimplemented".to_string());
        assert!(manager.contains_word(String::new()));
    }

    #[test]
    #[should_panic]
    fn construction_in_progress_for_define() {
        let mut manager = DictionaryManager::new(Language::English, "Unimplemented".to_string());
        assert_eq!(manager.define(String::new()).as_str(), Rc::new(Entry::new(String::new())).as_str());
    }

    #[test]
    fn dictionary_manager_name() {
        let mut manager = DictionaryManager::new(Language::English, "Unimplemented".to_string());
        assert_eq!(manager.get_name(), String::from("Unimplemented"));
        manager.rename(String::from("Promising"));
        assert_eq!(manager.get_name(), String::from("Promising"));
    }
}
