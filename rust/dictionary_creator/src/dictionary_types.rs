use std::collections::{ HashSet, HashMap };

pub type definitions_t = HashMap<String, HashSet<String>>;

pub type definer_t = fn(&str) -> definitions_t;
