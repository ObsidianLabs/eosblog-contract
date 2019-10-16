#include <eosblog.hpp>

ACTION eosblog::setconfig(string blogname, string description, string cover, string version, string metadata) {
  require_auth(get_self());
  
  config_index config_table(get_self(), get_self().value);
  auto config_singleton = config_table.get_or_create(get_self(), config { blogname, version, metadata });
  // update config
  config_singleton.blogname = blogname;
  config_singleton.description = description;
  config_singleton.cover = cover;
  config_singleton.version = version;
  config_singleton.metadata = metadata;
  // save config to table
  config_table.set(config_singleton, get_self());
}

ACTION eosblog::login() {
  require_auth(get_self());
}

ACTION eosblog::createpost(string title, string content, string cover, string author, name category, string metadata) {
  require_auth(get_self());

  // update post table
  post_index post_table(get_self(), get_self().value);
  post_table.emplace(get_self(), [&](auto& row) {
    uint32_t timestamp = current_time_point().sec_since_epoch();
    row.id = (uint64_t) timestamp;
    row.title = title;
    row.content = content;
    row.cover = cover;
    row.author = author;
    row.category = category;
    row.metadata = metadata;
    row.created_at = timestamp;
    row.updated_at = timestamp;
  });

  // update category table
  _add_category(category);
}

ACTION eosblog::updatepost(uint64_t id, string title, string content, string cover, string author, name category, string metadata) {
  require_auth(get_self());

  // update post table
  post_index post_table(get_self(), get_self().value);
  auto post_itr = post_table.find(id);
  // if the post exists
  if (post_itr != post_table.end()) {
    // preserve old category and sub it later
    name old_category = post_itr->category;
    // update post
    post_table.modify(post_itr, get_self(), [&](auto& row) {
      uint32_t timestamp = current_time_point().sec_since_epoch();
      row.title = title;
      row.content = content;
      row.cover = cover;
      row.author = author;
      row.category = category;
      row.metadata = metadata;
      row.updated_at = timestamp;
    });
    // update category table
    if (old_category != category) {
      _sub_category(old_category);
      _add_category(category);
    }
  }
}

ACTION eosblog::deletepost(uint64_t id) {
  require_auth(get_self());

  // update post table
  post_index post_table(get_self(), get_self().value);
  auto post_itr = post_table.find(id);
  if (post_itr != post_table.end()) {
    // preserve old category and sub it later
    name old_category = post_itr->category;
    post_itr = post_table.erase(post_itr);
    // update category table
    _sub_category(old_category);
  }
}

ACTION eosblog::clear() {
  require_auth(get_self());

  // remove config table
  config_index config_table(get_self(), get_self().value);
  config_table.remove();
}


// Private
void eosblog::_add_category(name category) {
  category_index category_table(get_self(), get_self().value);
  auto category_itr = category_table.find(category.value);
  if (category_itr == category_table.end()) {
    // does not exist
    category_table.emplace(get_self(), [&](auto& row) {
      row.categoryname = category;
      row.count = 1;
    });
  } else {
    // exists
    category_table.modify(category_itr, get_self(), [&](auto& row) {
      row.count = row.count + 1;
    });
  }
}

void eosblog::_sub_category(name category) {
  category_index category_table(get_self(), get_self().value);
  auto category_itr = category_table.find(category.value);
  if (category_itr != category_table.end()) {
    // if the category exists
    if (category_itr->count <= 1) {
      // delete category
      category_itr = category_table.erase(category_itr);
    } else {
      // reduce counter
      category_table.modify(category_itr, get_self(), [&](auto& row) {
        row.count = row.count - 1;
      });
    }
  }
}

EOSIO_DISPATCH(eosblog, (setconfig)(login)(clear)(createpost)(updatepost)(deletepost))
