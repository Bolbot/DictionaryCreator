use crate::dictionary_language;

use super::dictionary_entry::Entry;
use super::dictionary_language::Language;

use std::collections::{ HashMap, HashSet };
use std::rc::Rc;

trait DefaultEntrySorter {
}
type default_dictionary_type = HashMap<String, HashSet<Rc<Entry>>>;     // TODO: Maps and Sets with DefaultEntrySorter

pub struct Dictionary {
    language: Language,
    dictionary: default_dictionary_type,
    proper_nouns: default_dictionary_type
}
impl Dictionary {
    pub fn new(language: Language) -> Dictionary {
        Dictionary { language, dictionary: HashMap::new(), proper_nouns: HashMap::new() }
    }

    // TODO: set/map operations

    pub fn add_word(&self, word: String) {
        unimplemented!("under construction");
    }
    pub fn remove_word(&self, word: String) {
        unimplemented!("under construction");
    }
    pub fn add_proper_noun(&self, word: String) {
        unimplemented!("under construction");
    }
    pub fn remove_proper_nouns(&self) {
        unimplemented!("under construction");
    }

    pub fn lookup(&self, word: String) -> Rc<Entry> {
        unimplemented!("under construction");
    }
    pub fn total_words(&self) -> usize {
        let mut res = 0usize;
        for (_, v) in &self.dictionary {
            res += v.len();
        }
        res
    }

    // TODO: subset operations like get_undefined, get_top etc.

    pub fn get_language(&self) -> dictionary_language::Language {
        unimplemented!("under construction");
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    #[should_panic]
    fn construction_in_progress_for_add_word() {
        let english_dictionary = Dictionary::new(Language::English);
        english_dictionary.add_word("will".to_string());
    }

    #[test]
    #[should_panic]
    fn construction_in_progress_for_lookup() {
        let english_dictionary = Dictionary::new(Language::English);
        assert_eq!(english_dictionary.lookup("smith".to_string()).as_str(),
            Rc::new(Entry::new("smith".to_string())).as_str());
    }

    #[test]
    #[should_panic]
    fn construction_in_progress_for_get_language() {
        let english_dictionary = Dictionary::new(Language::English);
        assert_eq!(english_dictionary.get_language(), Language::English);
    }
}
