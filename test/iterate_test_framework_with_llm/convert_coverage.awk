/^ *[0-9]/ && !/:$/ {
    # Code line: starts with digits, doesn't end with ':'
    line = $0
    sub(/^ */, "", line)          # strip leading spaces
    split(line, parts, "|")       # split on |
    num = parts[1] + 0
    if (header != "") {
        if (first == "") first = num
        last = num
    }
    next
}

/^ *[^ ].*:$/ && /\|/ {
    # Case label (ends with ':' and has coverage '|' format) — skip without resetting
    next
}

# Blank line, header, comment, or other non-code: flush previous range, start new
{
    if (header != "") {
        print header ":" first "-" last
    }
    header = $0
    first = ""
    last = ""
}

END {
    if (header != "") {
        print header ":" first "-" last
    }
}
