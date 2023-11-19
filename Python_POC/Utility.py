import ast
from typing import List, Dict, Tuple

import AES
from Log import log, ERROR_COLOR, ACTION_COLOR
from Python_POC.Constants import *


def eval_list(response: str) -> List[int]:
    try:
        # Parse the string representation of the list into a Python AST (Abstract Syntax Tree)
        list_ast = ast.literal_eval(response)

        # Check if the parsed AST is a valid list
        if isinstance(list_ast, list):
            return list_ast
        else:
            raise ValueError("Input is not a valid list")

    except (SyntaxError, ValueError) as e:
        # Handle exceptions, such as SyntaxError or ValueError during evaluation
        print(f"Error evaluating list: {e}")
        # You can choose to return a default value or raise an exception based on your needs
        return []


def format_keys(aes_key: str, aes_iv: str) -> Tuple[bytes, bytes]:
    # String to bytes and remove extra "\"
    try:
        formatted_aes_key = AES.reformat_key_for_receivig(aes_key)
        formatted_aes_iv = AES.reformat_key_for_receivig(aes_iv)

        return formatted_aes_key, formatted_aes_iv
    except Exception:
        log(f"Error encoding keys to bytes: {aes_key} and {aes_iv}", ERROR_COLOR)
        raise ValueError("Input does not represent bytes.")


def print_path(nodes_map: Dict, random_path):
    # print the path (by the order of the nodes) in a nice way
    path = ""

    for node in random_path:
        path += str(nodes_map[node][NodeIndexes.SERVER_PORT_INDEX]) + " -> "

    path = path[:-4]

    log(f"Path: {path}", ACTION_COLOR)


def is_node(response: str) -> bool:
    have = len(response.split(SPLITER))
    need = NODE_INIT_FORMAT.count(SPLITER) + 1

    return have == need


def extract_connection_data(decrypted_response: str) -> List[str]:
    return decrypted_response.split(SPLITER)
