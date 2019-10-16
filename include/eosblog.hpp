#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>

using namespace std;
using namespace eosio;

CONTRACT eosblog : public contract {
  public:
    using contract::contract;

    // general actions
    ACTION setconfig(string blogname, string description, string cover, string version, string metadata);
    ACTION login();
    // posts actions
    ACTION createpost(string title, string content, string cover, string author, name category, string metadata);
    ACTION updatepost(uint64_t id, string title, string content, string cover, string author, name category, string metadata);
    ACTION deletepost(uint64_t id);
    // clear
    ACTION clear();

  private:
    void _add_category(name category);
    void _sub_category(name category);

    TABLE config {
      string  blogname;
      string  description;
      string  cover;
      string  version;
      string  metadata;
    };
    typedef singleton<"config"_n, config> config_index;

    // published posts
    TABLE post {
      uint64_t  id;
      string    title;
      string    content;
      string    cover;
      string    author;
      name      category;
      uint32_t  created_at;
      uint32_t  updated_at;
      string    metadata;

      uint64_t  primary_key() const { return id; }
      uint64_t  get_catagory() const { return category.value; }
    };
    typedef multi_index<"post"_n, post,
      indexed_by<"bycategory"_n, const_mem_fun<post, uint64_t, &post::get_catagory>>
    > post_index;

    TABLE category {
      name      categoryname;
      uint64_t  count;

      uint64_t primary_key() const { return categoryname.value; }
    };
    typedef multi_index<"category"_n, category> category_index;
    
    TABLE tag {
      name      tagname;
      uint64_t  count;
      
      uint64_t primary_key() const { return tagname.value; }
    };
    typedef multi_index<"tag"_n, tag> tag_index;
};
