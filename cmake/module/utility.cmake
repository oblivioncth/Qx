function(string_proper_case str return)
  string(SUBSTRING ${str} 0 1 FIRST_LETTER)
  string(SUBSTRING ${str} 1 -1 OTHER_LETTERS)
  string(TOUPPER ${FIRST_LETTER} FIRST_LETTER_UC)

  set(${return} "${FIRST_LETTER_UC}${OTHER_LETTERS}" PARENT_SCOPE)
endfunction()
