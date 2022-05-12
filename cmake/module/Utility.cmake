function(string_to_proper_case str return)
  string(SUBSTRING ${str} 0 1 FIRST_LETTER)
  string(SUBSTRING ${str} 1 -1 OTHER_LETTERS)
  string(TOUPPER ${FIRST_LETTER} FIRST_LETTER_UC)
  string(TOLOWER ${OTHER_LETTERS} OTHER_LETTERS_LC)

  set(${return} "${FIRST_LETTER_UC}${OTHER_LETTERS_LC}" PARENT_SCOPE)
endfunction()

function(create_header_guard prefix name return)
    # Replace all dashes and space with underscore, force uppercase
    string(REGEX REPLACE "[\r\n\t -]" "_" prefix_clean ${prefix})
    string(REGEX REPLACE "[\r\n\t -]" "_" name_clean ${name})
    string(TOUPPER ${name_clean} name_clean_uc)
    string(TOUPPER ${prefix_clean} prefix_clean_uc)

    set(${return} "${prefix_clean_uc}_${name_clean_uc}_H" PARENT_SCOPE)
endfunction()

function(get_subdirectory_list path return)
    file(GLOB path_children RELATIVE "${path}" "${path}/*")
    foreach(child ${path_children})
        if(IS_DIRECTORY "${path}/${child}")
            list(APPEND subdirs ${child})
        endif()
    endforeach()

    set(${return} "${subdirs}" PARENT_SCOPE)
endfunction()
