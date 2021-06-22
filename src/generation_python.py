import numpy as np

def get_dual_encoding(arc_permutation_encoding):
    return np.array([np.argsort(arc_permutation_encoding[0,:])+1,
                     np.argsort(arc_permutation_encoding[1,:])+1
                    ], dtype=np.float16)

def normalize_permutation_encoding(arc_permutation_encoding):
    assert set(arc_permutation_encoding[0,:])==set(arc_permutation_encoding[1,:])
    arc_permutation_encoding[0,:] = np.argsort(arc_permutation_encoding[0,:]).argsort()+1
    arc_permutation_encoding[1,:] = np.argsort(arc_permutation_encoding[1,:]).argsort()+1
    return arc_permutation_encoding.astype(np.float16)

## TODO handle the edge direction
def destabilize(arc_permutation_encoding, edge_index, left=True, up=True, expand=True):
    x1, x2 = arc_permutation_encoding[:,edge_index-1]
    arc_permutation_tmp = arc_permutation_encoding.copy()
    #x1, x2 = min(x1,x2), max(x1,x2)
    ##existing edge modification
    if left:
        arc_permutation_tmp[:,edge_index-1] = [x1-1.0*expand+0.5, x2]
        arc_permutation_tmp = np.insert(arc_permutation_tmp,
                                               edge_index-up,
                                               [x1, x1-1.0*expand+0.5],
                                               axis=1)
    else:
        arc_permutation_tmp[:,edge_index-1] = [x1, x2+expand-0.5]
        arc_permutation_tmp = np.insert(arc_permutation_tmp,
                                              edge_index-up,
                                              [x2+expand-0.5, x2],
                                              axis = 1)
    arc_permutation_tmp = normalize_permutation_encoding(arc_permutation_tmp)
    return arc_permutation_tmp


def destabilize_new(arc_permutation_encoding, edge_index, left=True, up=True, expand=True):
    x1, x2 = arc_permutation_encoding[:,edge_index-1]
    arc_permutation_tmp = arc_permutation_encoding.copy()
    arc_permutation_tmp[arc_permutation_tmp>=x1]+=1
    arc_permutation_tmp = np.insert(arc_permutation_tmp,
                                               edge_index-1,
                                               [x1+1, x1],
                                               axis=1)
    arc_permutation_tmp[0,edge_index]-=1
    return arc_permutation_tmp

def is_interleaving(arc1, arc2):
    return is_intersecting(arc1, arc2) and not (is_embedded(arc1,arc2) or is_embedded(arc2,arc1))


def is_embedded(arc1, arc2):
    ###
    ###ASYMMETRIC
    ###
    return np.max(arc1)<=np.max(arc2) and np.min(arc2)<=np.min(arc1)

def is_intersecting(arc1, arc2):
    return not(np.max(arc1)<=np.min(arc2) or np.max(arc2)<=np.min(arc1))

def get_crossing_number(arc_enc):
    crossnum = 0
    dual_enc = get_dual_encoding(arc_enc)
    for i,(a,b) in enumerate(arc_enc.T):
        possibilities = np.where((dual_enc.max(0)>i+1)*(dual_enc.min(0)<i+1))[0]
#         print(possibilities)
#         print(possibilities<max(a,b))
#         print(min(a,b)<possibilities)
        crossnum += ((possibilities<max(a,b))*(min(a,b)<=possibilities)).sum()
    return crossnum

def get_possible_switches_one_dir(arc_permutation_encoding):
    permutable_arcs_list = []
    for index in range(arc_permutation_encoding.shape[1]-1):
        arc1 = arc_permutation_encoding[:,index]
        arc2 = arc_permutation_encoding[:,index+1]
        if not is_interleaving(arc1, arc2):
            permutable_arcs_list.append((index+1, index+2))
    return permutable_arcs_list

def get_possible_switches(arc_permutation_encoding):
    horizontal = get_possible_switches_one_dir(arc_permutation_encoding)
    enc_dual = get_dual_encoding(arc_permutation_encoding)
    vertical = get_possible_switches_one_dir(enc_dual)
    ans = [(x, False) for x in horizontal] + [(x, True) for x in vertical]
    return ans

def switch_edges(arc_permutation_encoding, index_pair, vertical = False):
    ind1, ind2 = index_pair
    if vertical:
        dual_encoding = get_dual_encoding(arc_permutation_encoding)
        enc_dual = dual_encoding.copy()
        tmp = dual_encoding[:,ind1-1]
        enc_dual[:,ind1-1] = dual_encoding[:,ind2-1]
        enc_dual[:,ind2-1] = tmp
        enc = get_dual_encoding(enc_dual)
    else:
        enc = arc_permutation_encoding.copy()
        tmp = arc_permutation_encoding[:,ind1-1]
        enc[:,ind1-1] = arc_permutation_encoding[:,ind2-1]
        enc[:,ind2-1] = tmp
    return enc

def entangle_diag_fixed(arc_permutation_encoding, max_complexity=30):
    enc = arc_permutation_encoding.copy()
    move_list = []
    while enc.shape[1]<max_complexity:
        dual_enc = get_dual_encoding(enc)
        possible_switches =  get_possible_switches(enc)
        choose_destab = True if not possible_switches else np.random.rand()>0.9
        if choose_destab:
            row = np.random.randint(1, enc.shape[1]+1)
            f1, f2, f3 = np.random.rand(3)>0.5
            enc = destabilize(enc, row, f1, f2, f3)
            move_list.append(('destab', row, f1,f2,f3))
        else:
            index_pair, is_vertical = possible_switches[np.random.randint(0,
                                                          len(possible_switches))]
            enc = switch_edges(enc, index_pair, is_vertical)
            hor_str = 'vertical' if is_vertical else 'horizontal'
            move_list.append(('switch', hor_str, index_pair[0], index_pair[1]))
        #plt.figure()
        #draw_arc_diagram(enc)
        #print(enc)
    return enc, move_list



def entangle_diag_corner_complexity(arc_permutation_encoding, max_complexity=30):
    enc = arc_permutation_encoding.copy()
    move_list = []
    while enc.shape[1]<max_complexity:
        possible_switches =  get_possible_switches(enc)
        choose_destab = True if not possible_switches else np.random.rand()>0.9
        if choose_destab:
            row = np.random.randint(1, enc.shape[1]+1)
            f1, f2, f3 = np.random.rand(3)>0.5
            enc = destabilize(enc, row, f1, f2, f3)
            move_list.append(('destab', row, f1,f2,f3))
        else:
            further_diags = [switch_edges(enc, index_pair_, is_vertical_)
                             for index_pair_, is_vertical_ in possible_switches]
            
            crossing_complexities = [get_crossing_number(diag) for diag in further_diags]
            best_index = np.argmax(crossing_complexities)
            index_pair, is_vertical = possible_switches[best_index]
            enc = further_diags[best_index]
            hor_str = 'vertical' if is_vertical else 'horizontal'
            move_list.append(('switch', hor_str, index_pair[0], index_pair[1]))
        #plt.figure()
        #draw_arc_diagram(enc)
        #print(enc)
    return enc, move_list

def entangle_just_by_switches(enc_inp, num_steps=100):
    enc = enc_inp.copy()
    for step in range(num_steps):
        possible_switches =  get_possible_switches(enc)
        index_pair, is_vertical = possible_switches[np.random.randint(0,
                                                          len(possible_switches))]
        enc = switch_edges(enc, index_pair, is_vertical)
    return enc


def entangle_new(enc_inp, max_complexity, switch_max=9500):
    enc = enc_inp.copy()
    move_list = []
    while enc.shape[1]<max_complexity:
            row = np.random.randint(1, enc.shape[1]+1)
            f1, f2, f3 = np.random.rand(3)>0.5
            enc = destabilize(enc, row, f1, f2, f3)
            move_list.append(('destab', row, f1,f2,f3))
            
            
    for __ in range(switch_max):
        dual_enc = get_dual_encoding(enc)
        validity_flag = False
        while not validity_flag:
            index = np.random.randint(1, enc.shape[1])
            index_pair = (index, index+1)
            is_vertical = np.random.rand()>0.5
            if is_vertical:
                validity_flag = not is_interleaving(dual_enc[:,index-1], dual_enc[:,index])
            else:
                validity_flag = not is_interleaving(enc[:,index-1], enc[:,index])
        enc = switch_edges(enc, index_pair, is_vertical)
        hor_str = 'vertical' if is_vertical else 'horizontal'
        move_list.append(('switch', hor_str, index_pair[0], index_pair[1]))
        #plt.figure()
        #draw_arc_diagram(enc)
        #print(enc)
    return enc, move_list

