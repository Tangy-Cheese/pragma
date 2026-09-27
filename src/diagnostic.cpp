#include "diagnostic.hpp"

#include <iostream>

void DiagnosticCollector::add(size_t row_idx, const std::string& message, Severity severity){
    diagnostics.push_back({row_idx, message, severity});
}

bool DiagnosticCollector::has_errors() const{

    for (auto& diag: diagnostics){
        if (diag.severity == Severity::ERROR) return true;
    }

    return false;
}

void DiagnosticCollector::print_all() const{

    for (auto& diag: diagnostics){
        std::cerr << (diag.severity == Severity::ERROR ? "Error: " : "Warning: ") << diag.message
                   << " (row " << diag.row_idx << ")." << std::endl;
    }
}

const std::vector<Diagnostic>& DiagnosticCollector::get_diagnostics() const{
    return diagnostics;
}
