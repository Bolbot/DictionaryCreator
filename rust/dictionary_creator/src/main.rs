use dictionary_creator::dictionary_manager::DictionaryManager;
use dictionary_creator::dictionary_language::Language;

fn main() {
    let manager = DictionaryManager::new(Language::English, "not implemented yet".to_string());
    println!("It's {}. Come again later.", manager.get_name());
}
