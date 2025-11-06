// TODO: ExportOptions, probably something better than bitmasks

pub struct DictionaryExporter {
    // TODO: file stream handles
}
impl DictionaryExporter {
    // TODO: file stream opearations
    pub fn temporary_visibility_test(&self) -> bool { true }
    pub fn new() -> DictionaryExporter { DictionaryExporter {  } }
    pub fn all_operations(&self) { unimplemented!("Not even declared yet"); }
}

#[cfg(test)]
mod tests {
    use super::DictionaryExporter;

    #[test]
    #[should_panic]
    fn construction_in_progress_for_DictionaryExporter() {
        let temp = DictionaryExporter::new();
        assert!(temp.temporary_visibility_test());
        temp.all_operations();
    }
}
