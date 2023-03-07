#include "ada.h"
#include "ada/helpers.h"

namespace ada {

void url_base::set_hash(const std::string_view input) {
  if (input.empty()) {
    update_base_hash(std::nullopt);
    helpers::strip_trailing_spaces_from_opaque_path(*this);
    return;
  }

  std::string new_value;
  new_value = input[0] == '#' ? input.substr(1) : input;
  helpers::remove_ascii_tab_or_newline(new_value);
  update_base_hash(unicode::percent_encode(new_value, ada::character_sets::FRAGMENT_PERCENT_ENCODE));
}

bool url_base::set_pathname(const std::string_view input) {
  if (has_opaque_path) { return false; }
  update_base_pathname("");
  return parse_path(input);
}

bool url_base::set_username(const std::string_view input) {
  if (cannot_have_credentials_or_port()) { return false; }
  update_base_username(ada::unicode::percent_encode(input, character_sets::USERINFO_PERCENT_ENCODE));
  return true;
}

bool url_base::set_password(const std::string_view input) {
  if (cannot_have_credentials_or_port()) { return false; }
  update_base_password(ada::unicode::percent_encode(input, character_sets::USERINFO_PERCENT_ENCODE));
  return true;
}

void url_base::set_search(const std::string_view input) {
  if (input.empty()) {
    update_base_search(std::nullopt);
    helpers::strip_trailing_spaces_from_opaque_path(*this);
    return;
  }

  std::string new_value;
  new_value = input[0] == '?' ? input.substr(1) : input;
  helpers::remove_ascii_tab_or_newline(new_value);

  auto query_percent_encode_set = is_special() ?
    ada::character_sets::SPECIAL_QUERY_PERCENT_ENCODE :
    ada::character_sets::QUERY_PERCENT_ENCODE;

  update_base_search(ada::unicode::percent_encode(std::string_view(new_value), query_percent_encode_set));
}

bool url_base::set_port(const std::string_view input) {
  if (cannot_have_credentials_or_port()) { return false; }
  std::string trimmed(input);
  helpers::remove_ascii_tab_or_newline(trimmed);
  if (trimmed.empty()) { update_base_port(std::nullopt); return true; }
  // Input should not start with control characters.
  if (ada::unicode::is_c0_control_or_space(trimmed.front())) { return false; }
  // Input should contain at least one ascii digit.
  if (input.find_first_of("0123456789") == std::string_view::npos) { return false; }

  // Revert changes if parse_port fails.
  std::optional<uint32_t> previous_port = retrieve_base_port();
  parse_port(trimmed);
  if (is_valid) { return true; }
  update_base_port(previous_port);
  is_valid = true;
  return false;
}

ada_really_inline bool url_base::parse_path(std::string_view input) {
  ada_log("parse_path ", input);
  std::string tmp_buffer;
  std::string_view internal_input;
  if(unicode::has_tabs_or_newline(input)) {
    tmp_buffer = input;
    // Optimization opportunity: Instead of copying and then pruning, we could just directly
    // build the string from user_input.
    helpers::remove_ascii_tab_or_newline(tmp_buffer);
    internal_input = tmp_buffer;
  } else {
    internal_input = input;
  }

  // If url is special, then:
  if (is_special()) {
    if(internal_input.empty()) {
      update_base_pathname("/");
    } else if((internal_input[0] == '/') ||(internal_input[0] == '\\')){
      return helpers::parse_prepared_path(internal_input.substr(1), type, path);
    } else {
      return helpers::parse_prepared_path(internal_input, type, path);
    }
  } else if (!internal_input.empty()) {
    if(internal_input[0] == '/') {
      return helpers::parse_prepared_path(internal_input.substr(1), type, path);
    } else {
      return helpers::parse_prepared_path(internal_input, type, path);
    }
  } else {
    if(!base_hostname_has_value()) {
      update_base_pathname("/");
    }
  }
  return true;
}

} // namespace ada