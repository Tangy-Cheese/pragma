#pragma once

#include <cstddef>
#include <string>
#include <vector>

enum class Severity{
    ERROR,
    WARNING
};

struct Diagnostic{

    size_t row_idx;
    std::string message;
    Severity severity;
};

class DiagnosticCollector{

public:

    /**
     * @brief Records a diagnostic instead of exiting immediately, so
     *  checking can continue and every error gets reported in one pass.
     */
    void add(size_t row_idx, const std::string& message, Severity severity = Severity::ERROR);

    /**
     * @brief Whether any Severity::ERROR diagnostic has been recorded.
     *
     * @return True if at least one error was collected.
     */
    bool has_errors() const;

    /**
     * @brief Prints every collected diagnostic to std::cerr, in the order
     *  they were recorded.
     */
    void print_all() const;

    /**
     * @brief Read-only access to every diagnostic collected so far.
     *
     * @return Vector of collected diagnostics.
     */
    const std::vector<Diagnostic>& get_diagnostics() const;

private:
    std::vector<Diagnostic> diagnostics;
};
