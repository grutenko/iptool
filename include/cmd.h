#ifndef cmd_h
#define cmd_h

#include "core.h"

typedef int (*cmd_proc_p)(int argc, char **argv);
typedef void (*cmd_help_proc_p)(void);

struct cmd_struct {
  const char *name;
  const char *descr;
  cmd_proc_p proc;
  cmd_help_proc_p help_proc;
};

struct cmd_struct *list_cmd();

/**
 * @brief Find a command by name.
 *
 * @param name name of command
 * @return pointer to command structure or NULL if not found
 */
struct cmd_struct *find_cmd(const char *name);
/**
 * @brief Parse IP addresses from command line arguments.
 *
 * Parse ip address list from command line in many types if error print message
 * and exit. Call this function with arguments start from first ip address
 *
 * @param[in] argc number of arguments
 * @param[in] argv array of arguments
 */
void parse_ips(int argc, char **argv, iap_t **root);

void cmd_filter_help();
void cmd_inflate_help();
void cmd_deflate_help();
void cmd_invert_help();

/**
 * @brief Invert the addresses in the tree.
 *
 * Command procedure to invert the addresses in the tree.
 *
 * @param r root of tree
 * @param out output stream
 * @return 0 on success, -1 on error
 */
int cmd_invert(int argc, char **argv);
/**
 * @brief Filter the addresses in the tree.
 *
 * Command procedure to filter the addresses in the tree.
 *
 * @param r root of tree
 * @param out output stream
 * @return 0 on success, -1 on error
 */
int cmd_filter(int argc, char **argv);
/**
 * @brief Deflate the addresses in the tree.
 *
 * Command procedure to deflate (compress by cidr) the addresses in the tree.
 *
 * @param r root of tree
 * @param out output stream
 * @return 0 on success, -1 on error
 */
int cmd_deflate(int argc, char **argv);
/**
 * @brief Inflate the addresses in the tree.
 *
 * Command procedure to inflate (expand) the addresses in the tree.
 *
 * @param r root of tree
 * @param out output stream
 * @return 0 on success, -1 on error
 */
int cmd_inflate(int argc, char **argv);
/**
 * @brief Help the user.
 *
 * Command procedure to help the user.
 *
 * @param r root of tree
 * @param out output stream
 * @return 0 on success, -1 on error
 */
int cmd_help(int argc, char **argv);
/**
 * @brief List the addresses in the tree.
 *
 * Command procedure to list the addresses in the tree.
 *
 * @param r root of tree
 * @param out output stream
 * @return 0 on success, -1 on error
 */
int cmd_list(int argc, char **argv);

#endif
