pub fn get_input_file_path() -> String {
    unimplemented!("under construction");
}

pub fn get_output_file_path() -> String {
    unimplemented!("under construction");
}

#[cfg(test)]
mod tests {
    use super::get_input_file_path;
    use super::get_output_file_path;

    #[test]
    #[should_panic]
    fn construction_in_progress_for_get_input_file_path() {
        assert_eq!(get_input_file_path(), String::new());
    }

    #[test]
    #[should_panic]
    fn construction_in_progress_for_get_output_file_path() {
        assert_eq!(get_output_file_path(), String::new());
    }
}
