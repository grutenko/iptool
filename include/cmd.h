#ifndef cmd_h
#define cmd_h

typedef int (*cmd_proc_p)(int argc, char **argv);
typedef void (*cmd_help_proc_p)(void);

struct cmd_struct {
  const char *name;
  const char *descr;
  cmd_proc_p proc;
  cmd_help_proc_p help_proc;
};

/**
 * @brief Find a command by name.
 *
 * @param name name of command
 * @return pointer to command structure or NULL if not found
 */
struct cmd_struct *find_cmd(const char *name);

void cmd_filter_help_proc();
void cmd_inflate_help_proc();
void cmd_deflate_help_proc();
void cmd_invert_help_proc();

/**
 * @brief Invert the addresses in the tree.
 *
 * Command procedure to invert the addresses in the tree.
 *
 * @param r root of tree
 * @param out output stream
 * @return 0 on success, -1 on error
 */
int cmd_invert_proc(int argc, char **argv);
/**
 * @brief Filter the addresses in the tree.
 *
 * Command procedure to filter the addresses in the tree.
 *
 * @param r root of tree
 * @param out output stream
 * @return 0 on success, -1 on error
 */
int cmd_filter_proc(int argc, char **argv);
/**
 * @brief Deflate the addresses in the tree.
 *
 * Command procedure to deflate (compress by cidr) the addresses in the tree.
 *
 * @param r root of tree
 * @param out output stream
 * @return 0 on success, -1 on error
 */
int cmd_deflate_proc(int argc, char **argv);
/**
 * @brief Inflate the addresses in the tree.
 *
 * Command procedure to inflate (expand) the addresses in the tree.
 *
 * @param r root of tree
 * @param out output stream
 * @return 0 on success, -1 on error
 */
int cmd_inflate_proc(int argc, char **argv);
/**
 * @brief Help the user.
 *
 * Command procedure to help the user.
 *
 * @param r root of tree
 * @param out output stream
 * @return 0 on success, -1 on error
 */
int cmd_help_proc(int argc, char **argv);
/**
 * @brief List the addresses in the tree.
 *
 * Command procedure to list the addresses in the tree.
 *
 * @param r root of tree
 * @param out output stream
 * @return 0 on success, -1 on error
 */
int cmd_list_proc(int argc, char **argv);

#endif
