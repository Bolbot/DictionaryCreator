pub fn parse_json_to_definitions_set(in_json: &str) -> std::collections::HashSet<String> {
    unimplemented!("Under construction");
}

pub fn parse_json_to_definitions_map(in_json: &str) -> std::collections::HashMap<String, std::collections::HashSet<String>> {
    unimplemented!("Under construction");
}

#[cfg(test)]
mod tests {
    use std::collections::HashSet;
    use std::collections::HashMap;

    use super::parse_json_to_definitions_set;
    use super::parse_json_to_definitions_map;

    #[test]
    #[should_panic]
    fn construction_in_progress_for_parse_json_to_definitions_set() {
        assert_eq!(parse_json_to_definitions_set("unimplemented"), HashSet::new());
    }

    #[test]
    #[should_panic]
    fn construction_in_progress_for_parse_json_to_definitions_map() {
        assert_eq!(parse_json_to_definitions_map("unimplemented"), HashMap::new());
    }
}
