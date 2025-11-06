pub fn get(inURI: &str) -> String {
    unimplemented!("Construction in progress");
}

#[cfg(test)]
mod tests {
    use super::get;

    #[test]
    #[should_panic]
    fn construction_in_progress_for_get() {
        assert_eq!(get("unimplemented!"), "panics");
    }
}
