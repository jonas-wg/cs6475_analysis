# runs the range-analysis plugin over one MLIR input and checks the annotated
# listing.
#
# Expected:
#   -DMLIR_OPT=
#   -DPLUGIN=
#   -DINPUT=
#   -DCHECKS=

foreach(required MLIR_OPT PLUGIN INPUT CHECKS)
  if(NOT DEFINED ${required})
    message(FATAL_ERROR "RunTest.cmake: -D${required}= is required")
  endif()
endforeach()

execute_process(
  COMMAND "${MLIR_OPT}"
          "--load-pass-plugin=${PLUGIN}"
          "--pass-pipeline=builtin.module(range-analysis)"
          "${INPUT}"
  OUTPUT_VARIABLE ignored_ir
  ERROR_VARIABLE annotated
  RESULT_VARIABLE status)

if(NOT status EQUAL 0)
  message(FATAL_ERROR "mlir-opt failed (${status}):\n${annotated}")
endif()

# file(STRINGS) leaves a trailing CR on a file checked out with Windows line
# endings, which is easy to do under WSL2. Normalize rather than fail.
file(STRINGS "${CHECKS}" expectations)

set(failures "")
foreach(expectation IN LISTS expectations)
  string(REGEX REPLACE "\r$" "" expectation "${expectation}")

  if(expectation STREQUAL "" OR expectation MATCHES "^#")
    continue()
  endif()

  # A leading '!' inverts the check.
  set(inverted FALSE)

  if(expectation MATCHES "^!")
    set(inverted TRUE)
    string(SUBSTRING "${expectation}" 1 -1 expectation)
  endif()

  string(FIND "${annotated}" "${expectation}" position)

  if(inverted AND NOT position EQUAL -1)
    string(APPEND failures "  unexpected: ${expectation}\n")
  elseif(NOT inverted AND position EQUAL -1)
    string(APPEND failures "  missing: ${expectation}\n")
  endif()
endforeach()

if(NOT failures STREQUAL "")
  message(FATAL_ERROR
    "Annotated output did not contain every expected fact:\n${failures}"
    "--- actual ---\n${annotated}")
endif()

message(STATUS "range-analysis: all expected facts present")

