# Rust

This directory is for Rust counterparts of the C++ project from the neighbor cpp/ directory.

## Structure

It's comprised of two main projects:

* **auxiliary** for targets interacting with external dependencies and file system:
  * connections interacts with *curl*
  * json_parser parses jsons with *nlohmann/json*
  * reges_parser parses files with *PCRE2*
  * fs_manager interacts with file system
* **dictionary_creator** a.k.a. the core of the project:
  * dictionary module stores the dictionaries and filters out the proper nouns
  * dictionary_creator module parses files with *regex_parser*
  * dictionary_definer module downloads definitions via *connections* and parses them with *json_parser*
  * dictionary_entry module is the lowest level, stores one word with all related to it
  * dictionary_exporter module exports the dictionary into plain files with readable formatting
  * dictionary_language is a bundle of language-specific settings
  * dictionary_types is a list of project-specific type aliases
  * **dictionary_manager** is the facade facing the userts, it comprises dictionary, creator, and exporter
  * *main* is a tiny command-line executable for POC and ad-hoc testing, uses *fs_manager* to interact with user
