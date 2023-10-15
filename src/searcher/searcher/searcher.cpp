#include <common/parser.hpp>
#include <indexer/indexer.hpp>
#include <searcher/searcher.hpp>

#include <picosha2.h>

#include <cmath>
#include <fstream>
#include <string>
#include <vector>

namespace {

double tf_idf(double tf, double df, double N) { return tf * log(N / df); }

} // namespace

namespace searcher {

Result search(const std::string &query, TextIndexAccessor &index_accessor) {

  std::vector<std::vector<std::string>> terms;
  parser::parse_text(query, terms, index_accessor.config());

  TermInfos term_info;
  for (const auto &items : terms) {
    for (const auto &term : items) {
      term_info = index_accessor.get_term_infos(term, false);
    }
  }

  std::map<size_t, double> results;
  const auto total_docs = static_cast<double>(index_accessor.total_docs());
  for (const auto &[key, value] : term_info.entries_) {
    for (const auto &[k, v] : value) {
      const size_t doc_id = k;
      const auto tf = static_cast<double>(v[0]);
      const auto df = static_cast<double>(v[1]);
      const double tmp_res = tf_idf(tf, df, total_docs);
      results[doc_id] += tmp_res;
    }
  }

  return sort_results(results);
}

TermInfos
TextIndexAccessor::get_term_infos(const std::string &term, bool testflag) {

  std::ifstream file(
      path_ / "index" / "entries" / indexer::hashing_term(term, testflag));

  std::string term_entries;
  while (std::getline(file, term_entries)) {
    std::vector<std::string> list_terms = parser::delete_spaces(term_entries);
    const std::string word = list_terms[0];
    const size_t doc_frequency = std::stoi(list_terms[1]);
    size_t i = 2;
    while (i < list_terms.size()) {
      const size_t doc_id = std::stoi(list_terms[i]);
      i++;
      const size_t term_frequency = std::stoi(list_terms[i]);
      term_infos_.entries_[word][doc_id].push_back(term_frequency);
      term_infos_.entries_[word][doc_id].push_back(doc_frequency);
      i += term_frequency + 1;
    }
  }

  return term_infos_;
}

std::string TextIndexAccessor::load_document(size_t document_id) const {
  std::string text;
  std::ifstream file(path_ / "index" / "docs" / std::to_string(document_id));
  std::getline(file, text);

  return text;
}

size_t TextIndexAccessor::total_docs() const {
  size_t total_docs = 0;
  std::ifstream file(path_ / "index" / "docs" / "total_docs");
  file >> total_docs;

  return total_docs;
}

Result sort_results(const std::map<size_t, double> &tmp_results) {
  Result sorted;
  // std::copy(
  //     tmp_results.begin(),
  //     tmp_results.end(),
  //     std::back_inserter<std::vector<IdScore>>(sorted.results_));

  tmp_results.size();
  // std::sort(
  //     sorted.results_.begin(),
  //     sorted.results_.end(),
  //     [](const pair &l, const pair &r) {
  //       if (l.second != r.second) {
  //         return l.second > r.second;
  //       }

  //       return l.first > r.first;
  //     });

  return sorted;
}

} // namespace searcher