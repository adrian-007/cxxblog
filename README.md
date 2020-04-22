## cxxblog - Blogging Web Application written in C++17

### Features

1. Post management
   * Publish / hide posts
   * Post modification history (drafts)
   * Uses Markdown markup language for post formatting
   * Supports additional expressions (for ex. image handling)
2. Attachment management
   * Stores files inside a database, creates a static lazy cache in filesystem for better performance
3. Editor info
   * About section
   * Contact info section
   * Custom or auto-generated avatars
4. About page

---

Default theme is based on Bootstrap 3, views are based on Wt's XML templates.

Markdown is provided by libcmark-gfm.
