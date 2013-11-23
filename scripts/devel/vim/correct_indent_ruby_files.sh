#!/bin/bash

source "$( cd "$( dirname "${BASH_SOURCE[0]}" )/../.." && pwd )"/configuration.sh

echo
echo "This script will correct indent ruby files in $CLONDIKE_SIMPLE_RUBY_DIRECTOR using the VIM."
echo
echo "You should have installed:"
echo "  vim"
echo "  vim-ruby (https://github.com/vim-ruby/vim-ruby.git)"
echo
echo "and properly have setted ~/.vimrc"
echo

function indent {
  find $CLONDIKE_SIMPLE_RUBY_DIRECTOR -type f -iname '*.rb' -exec vim -s $CLONDIKE_SCRIPTS/devel/vim/correct_indent.vim "{}" \;
}

read -r -p "Are you sure to continue now? [y/N] " response
case $response in
  [yY][eE][sS]|[yY])
    indent
    ;;
  *)
    ;;
esac
