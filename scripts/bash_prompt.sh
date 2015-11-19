#!/bin/bash
# Script for set Clondike prompt
# Autor: Jiri Rakosnik
#           edited by Zdenek Novy

# Save result value of the last command
CC=$?

# Function calling by every command to color last sharp
#  by return code of the last command
if [ "$1" = "defPS1" ] ; then
    [ $CC -eq 0 ] && DLR="\[\e[33;1m\]" || DLR="\[\e[31;1m\]"
	if [[ ${EUID} == 0 && -d /clondike/pen/nodes && -d /clondike/ccn/nodes ]] ; then
        PS1="\[\033[01;31m\]$(cut -d: -f2 /clondike/ccn/listen) $(cat /clondike/pen/nodes/count)/$(cat /clondike/ccn/nodes/count)\[\033[01;34m\] \W $DLR\\$\[\033[00m\] "
	else
		PS1='\[\033[01;32m\]\u@\h\[\033[01;34m\] \w $DLR\\$\[\033[00m\] '
	fi
    return 0;
fi



# Change the window title of X terminals 
case ${TERM} in
	xterm*|rxvt*|Eterm|aterm|kterm|gnome*|interix)
		PROMPT_COMMAND='echo -ne "\033]0;${USER}@${HOSTNAME%%.*}:${PWD/$HOME/~}\007"'
		;;
	screen)
		PROMPT_COMMAND='echo -ne "\033_${USER}@${HOSTNAME%%.*}:${PWD/$HOME/~}\033\\"'
		;;
esac

use_color=false

# Set colorful PS1 only on colorful terminals.
# dircolors --print-database uses its own built-in database
# instead of using /etc/DIR_COLORS.  Try to use the external file
# first to take advantage of user additions.  Use internal bash
# globbing instead of external grep binary.
safe_term=${TERM//[^[:alnum:]]/?}   # sanitize TERM
match_lhs=""
[[ -f ~/.dir_colors   ]] && match_lhs="${match_lhs}$(<~/.dir_colors)"
[[ -f /etc/DIR_COLORS ]] && match_lhs="${match_lhs}$(</etc/DIR_COLORS)"
[[ -z ${match_lhs}    ]] \
	&& type -P dircolors >/dev/null \
	&& match_lhs=$(dircolors --print-database)
[[ $'\n'${match_lhs} == *$'\n'"TERM "${safe_term}* ]] && use_color=true

if ${use_color} ; then
	# Enable colors for ls, etc.  Prefer ~/.dir_colors #64489
	if type -P dircolors >/dev/null ; then
		if [[ -f ~/.dir_colors ]] ; then
			eval $(dircolors -b ~/.dir_colors)
		elif [[ -f /etc/DIR_COLORS ]] ; then
			eval $(dircolors -b /etc/DIR_COLORS)
		fi
	fi
    PROMPT_COMMAND="source ${BASH_SOURCE[0]} defPS1"
    # Old way without colors sharp by result of the last command + if from function defPS1
    #PS1='\[\033[01;31m\]$(cut -d: -f2 /clondike/ccn/listen) $(paste -d/ /clondike/{pen,ccn}/nodes/count)\[\033[01;34m\] \W \$\[\033[00m\] '

	alias ls='ls --color=auto'
	alias grep='grep --colour=auto'
else
	if [[ ${EUID} == 0 ]] ; then
		# show root@ when we don't have colors
		PS1='\u@\h \W \$ '
	else
		PS1='\u@\h \w \$ '
	fi
fi

# Try to keep environment pollution down.
unset use_color safe_term match_lhs


