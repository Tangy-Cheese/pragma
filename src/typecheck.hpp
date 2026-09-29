#pragma once

#include "diagnostic.hpp"
#include "parser.hpp"
#include <string>
#include <unordered_map>
#include <optional>
#include <vector>

struct func_signature{
    TokenType return_type;
    std::vector<TokenType> param_types;
};

/**
 * @brief Builds a table of every function's signature, before any body is
 *  checked, so a call can refer to a function defined later in the file.
 *
 * @param funcs Every parsed function, in source order.
 * @return Map from function name to its signature. On duplicate names the
 *  first definition is kept.
 */
std::unordered_map<std::string, func_signature> collect_signatures(const std::vector<std::unique_ptr<func_node>>& funcs);

class TypeChecker{

public:

    TypeChecker(func_node*, const std::unordered_map<std::string, func_signature>& signatures = {});
    ~TypeChecker();

    /**
     * @brief Runs check_func on member.
     */
    void checker();

    /**
     * @brief Read-only access to the type environment built up during checking.
     *
     * @return Map from variable name to inferred/declared type.
     */
    const std::unordered_map<std::string, TokenType>& get_env() const;

    /**
     * @brief Read-only access to the array element types built up during
     *  checking, keyed by array variable name.
     *
     * @return Map from array variable name to element type.
     */
    const std::unordered_map<std::string, TokenType>& get_array_env() const;

    /**
     * @brief OpenMP clauses inferred for each parallel_node during type
     *  checking (e.g. "reduction(+:sum)"), keyed by node pointer. Populated
     *  by check_parallel.
     *
     * @return Map from parallel_node pointer to its inferred clause strings.
     */
    const std::unordered_map<parallel_node*, std::vector<std::string>>& get_parallel_clauses() const;

    /**
     * @brief Diagnostics collected during checking, in place of exit(1) —
     *  every error found is recorded here rather than killing the process
     *  on the first one.
     *
     * @return Collector holding every diagnostic recorded so far.
     */
    const DiagnosticCollector& get_diagnostics() const;

private:
    func_node* root;
    std::unordered_map<std::string, TokenType> env;
    std::unordered_map<std::string, TokenType> array_env;
    std::unordered_map<parallel_node*, std::vector<std::string>> parallel_clauses;
    std::unordered_map<std::string, func_signature> signatures;
    DiagnosticCollector diagnostics;

    /**
     * @brief Type checks a function: binds param types into env, then checks
     *  the return expression against the declared return type.
     */
    void check_func(func_node*);

    /**
     * @brief Type checks a list of statements, growing env as locals are assigned.
     */
    void check_stmts(std::vector<std::unique_ptr<AST_node>>&);

    /**
     * @brief Type checks a for loop: checks its range bounds, binds the loop
     *  variable as int, then checks its body statements.
     */
    void check_for(for_node*);

    /**
     * @brief Type checks a parallel loop: same as check_for for now. OMP
     *  safety analysis (reduction detection, disallowed scalar writes) is a
     *  separate later pass, not part of type checking.
     */
    void check_parallel(parallel_node*);

    /**
     * @brief Type checks an out statement and records the printed value's
     *  type on the node, so codegen can pick the printf format.
     */
    void check_print(print_node*);

    /**
     * @brief Recursively type checks an expression node.
     *
     * @return Resulting type of the expression, or nullopt on error.
     */
    std::optional<TokenType> check_expr(AST_node*);
};

