#include <common/parser.hpp>
#include <indexer/indexer.hpp>

#include <picosha2.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace indexer {

void IndexBuilder::add_document(size_t document_id, const std::string &text) {

  index_.docs.insert({document_id, text});

  std::vector<std::vector<std::string>> ngrams;

  parser::parse_text(
      text, stop_words, ngrams, ngram_min_length, ngram_max_length);

  for (size_t pos = 0; pos < ngrams.size(); ++pos) {
    for (const auto &word : ngrams[pos]) {
      index_.entries[word][document_id].insert(pos);
    }
  }
}

void TextIndexWriter::write(std::string path, Index index_) {

  std::filesystem::create_directories(path);

  if (!index_.docs.empty()) {
    const std::string path_docs = path + "/docs/";
    std::filesystem::create_directories(path_docs);
    for (const auto &[id, str] : index_.docs) {
      std::ofstream file(path_docs + std::to_string(id));
      file << str;
    }
  }

  if (!index_.entries.empty()) {
    const std::string path_entries = path + "/entries/";
    std::filesystem::create_directories(path_entries);
    for (const auto &[term, term_info] : index_.entries) {
      std::string hash_term;
      picosha2::hash256_hex_string(term, hash_term);
      std::ofstream file(path_entries + hash_term.substr(0, 6));
      file << term << ' ';
      file << term_info.size() << ' ';
      for (const auto &[doc_id, info] : term_info) {
        file << doc_id << ' ';
        file << info.size() << ' ';
        for (const auto &i : info) {
          file << i << ' ';
        }
      }
      file.close();
    }
  }
}

} // namespace indexer