import numpy as np
import matplotlib.pyplot as plt
from src.generation_python import get_dual_encoding

def draw_arc_diagram(arc_permutation_encoding):
    for y in range(arc_permutation_encoding.shape[1]):
        x1, x2 = arc_permutation_encoding[:,y]
        plt.plot([x1,x2], [-y-1,-y-1], 'black')
    vertical_edges_encoding = get_dual_encoding(arc_permutation_encoding)
    for x in range(vertical_edges_encoding.shape[1]):
        y1, y2 = vertical_edges_encoding[:,x]
        plt.plot([x+1,x+1], [-y1,-y2], 'black')
    plt.grid()
    

def draw_by_history(init_diag, history):
    enc = init_diag.copy()  
    for move in history:
        plt.figure()
        if move[0] == 'destab':
            __, row, f1, f2, f3 = move
            enc = destabilize(enc, row, f1, f2, f3)        
        else:
            movetype, hor_str, index0, index1 = move
            assert movetype == 'switch'
            is_vertical = hor_str=='vertical' 
            enc = switch_edges(enc, (index0, index1), is_vertical)
        draw_arc_diagram(enc)