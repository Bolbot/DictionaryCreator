use super::dictionary_types;

pub struct Entry {
    word: String,
    definitions: dictionary_types::definitions_t,
    encounters: usize,
    defined: bool
}
impl Entry {
    pub fn new(in_word: String) -> Entry {
        Entry { word: in_word, definitions: dictionary_types::definitions_t::new() , encounters: 1, defined: false }
    }
    pub fn get_word(&self) -> String {
        self.word.clone()
    }
    pub fn get_definitions(&self) -> dictionary_types::definitions_t {
        self.definitions.clone()
    }
    pub fn is_defined(&self) -> bool {
        self.defined
    }
    pub fn define(&mut self, definer: dictionary_types::definer_t) {
        if !self.defined {
            self.definitions = definer(&self.word);
            if self.definitions.len() > 0 {
                self.defined = true;
            }
        }
    }
    pub fn get_counter(&self) -> usize {
        self.encounters
    }
    pub fn increment_counter(&mut self) {
        self.encounters += 1;
    }
    pub fn increment_counter_by(&mut self, arg: usize) {
        self.encounters += arg;
    }
    pub fn as_str(&self) -> &str {
        &self.word
    }
}

#[cfg(test)]
mod tests {
    use std::collections::HashSet;

    use super::*;

    #[test]
    fn dictionary_entry_creation() {
        let raw_word = "antidisestablismentarianism";
        let entry = Entry::new(raw_word.to_string());

        assert_eq!(entry.as_str(), raw_word);
        assert_eq!(entry.get_counter(), 1);
        assert_eq!(entry.get_definitions().len(), 0);
        assert_eq!(entry.is_defined(), false);
        assert_eq!(entry.get_word(), String::from(raw_word));
    }

    #[test]
    fn dictionary_entry_counter() {
        let mut entry = Entry::new("scientific".to_string());

        entry.increment_counter();
        assert_eq!(entry.get_counter(), 2);

        entry.increment_counter_by(10);
        assert_eq!(entry.get_counter(), 12);

        entry.increment_counter_by(1_000_000_000_000);
        assert_eq!(entry.get_counter(), 1_000_000_000_012);
    }

    #[test]
    fn dictionary_entry_defined() {
        let pseudodefiner = |any_word: &str| {
            let mut res = dictionary_types::definitions_t::new();
            res.insert("A".to_string(), HashSet::from([ "B".to_string(), "C".to_string(), "D".to_string() ]));
            res.insert("One".to_string(), HashSet::from([ "Two".to_string(), "Three".to_string(), "Four".to_string() ]));
            res
        };
        let pseudodefinitions = pseudodefiner("whatever");
        assert_eq!(pseudodefiner("argument is disregarded"), pseudodefinitions);

        let mut entry = Entry::new("volcano".to_string());
        entry.define(pseudodefiner);
        assert!(entry.is_defined());
        assert_eq!(entry.get_definitions(), pseudodefinitions);
        assert_eq!(entry.get_counter(), 1);
    }
}
