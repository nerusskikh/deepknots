import numpy as np
import ctypes
from multiprocessing import Pool
import gc
import os

from tensorflow.python.keras.utils.data_utils import Sequence



def init(movetype):
    if movetype == 'external':
        dll_path = os.path.join(os.getcwd(), 'src','generator_external.so')
    elif movetype == 'internal':
        dll_path = os.path.join(os.getcwd(), 'src','generator_internal.so')
    else:
        raise ValueError("Move type must be 'internal' or 'external'")
    
    global complication_dll
    complication_dll = ctypes.cdll.LoadLibrary(dll_path)
    complication_dll.complicate_wrapper.argtypes = [ctypes.c_int,
                                         ctypes.c_int,
                                         ctypes.c_int,
                                         np.ctypeslib.ndpointer(dtype = np.intc),
                                         np.ctypeslib.ndpointer(dtype = np.intc),
                                         np.ctypeslib.ndpointer(dtype = np.intc),
                                         np.ctypeslib.ndpointer(dtype = np.intc)]
    complication_dll.complicate_wrapper.restype  = ctypes.c_void_p


    complication_dll.gradual_complicate_wrapper.argtypes = [
                                         ctypes.c_int,
                                         ctypes.c_int,
                                         ctypes.c_int,
                                         np.ctypeslib.ndpointer(dtype = np.intc),
                                         np.ctypeslib.ndpointer(dtype = np.intc),
                                         np.ctypeslib.ndpointer(dtype = np.intc),
                                         np.ctypeslib.ndpointer(dtype = np.intc),
                                         np.ctypeslib.ndpointer(dtype = np.intc),
                                         np.ctypeslib.ndpointer(dtype = np.intc)]
    complication_dll.gradual_complicate_wrapper.restype  = ctypes.c_void_p


def entangle_c(label, size, switch_steps):
    toprow = np.empty(size, dtype=np.intc)
    bottomrow = np.empty(size, dtype=np.intc)
    toprow_dual = np.empty(size, dtype=np.intc)
    bottomrow_dual = np.empty(size, dtype=np.intc)
    complication_dll.complicate_wrapper(label, size, switch_steps,
                                        toprow, bottomrow,
                                       toprow_dual, bottomrow_dual)
    return np.array([toprow, bottomrow,
                     bottomrow_dual,
                     toprow_dual,
                    ])

def entangle_c_no_dual(label, size, switch_steps):
    toprow = np.empty(size, dtype=np.intc)
    bottomrow = np.empty(size, dtype=np.intc)
    toprow_dual = np.empty(size, dtype=np.intc)
    bottomrow_dual = np.empty(size, dtype=np.intc)
    complication_dll.complicate_wrapper(label, size, switch_steps,
                                        toprow, bottomrow,
                                       toprow_dual, bottomrow_dual)
    return np.array([toprow, bottomrow,
                     bottomrow_dual,
                     toprow_dual,
                    ])

def entangle_more_c(diag, max_complexity, switch_steps):
    current_complexity = diag.shape[1]
    assert current_complexity<=max_complexity
    toprow = np.empty(max_complexity, dtype=np.intc)
    bottomrow = np.empty(max_complexity, dtype=np.intc)
    toprow_dual = np.empty(max_complexity, dtype=np.intc)
    bottomrow_dual = np.empty(max_complexity, dtype=np.intc)
    complication_dll.gradual_complicate_wrapper(current_complexity, max_complexity, switch_steps,
                                        diag[1].astype(np.intc).copy(),
                                        diag[0].astype(np.intc).copy(),
                                        toprow, bottomrow,
                                       toprow_dual, bottomrow_dual)
    return np.array([toprow, 
                     bottomrow,
                     bottomrow_dual,
                     toprow_dual,
                    ], dtype=np.float32)

class Complicator_c():
    def __init__(self, complexity=20, switch_steps=300):
        self.complexity = complexity
        self.switch_steps = switch_steps
    def __call__(self, label):
        return entangle_c(label, self.complexity, self.switch_steps).T
    
class Complicator_c_gradual():
    def __init__(self, complexity=20, switch_steps=300):
        self.complexity = complexity
        self.switch_steps = switch_steps
    def __call__(self, diag):
        return entangle_more_c(diag.T, self.complexity, self.switch_steps).T
    
    
class data_generator_curriculum():
    def __init__(self, min_complexity=15,
                 max_complexity=30,
                 switch_steps=10000,
                 n_cores=24,
                 n_classes=36,

                ):
        self.n_classes = n_classes
        self.min_complexity = min_complexity
        self.max_complexity = max_complexity
        self.switch_steps = switch_steps
        self.pool = Pool(n_cores)
    def generate_data(self, n):
        batch_complexity = np.random.randint(self.min_complexity,self.max_complexity)
        indexes = np.random.randint(0, self.n_classes, n)
        compl = Complicator_c(batch_complexity,self.switch_steps)
        X = np.stack(self.pool.map(compl, indexes))
        y = np.zeros((n, self.n_classes), np.float32)
        
        y[range(n),indexes]=1.
        return X,y
    def stream(self, batch_size):
        while True:
            X,y = self.generate_data(batch_size)
            yield X,y
            
class data_generator_gradual(Sequence):
    def __init__(self,
                 buffer_size = 36*3*100,
                 batch_size = 36*3,
                 complexity=30,
                 switch_steps=100,
                 init_switch_steps=10,
                 n_cores=24,
                 n_classes=36
                ):

        self.buffer_size=buffer_size
        self.batch_size=batch_size
        self.n_classes = n_classes
        assert not self.buffer_size%self.batch_size
        assert not self.batch_size%self.n_classes
        self.complexity = complexity
        self.switch_steps = switch_steps
        self.init_switch_steps = init_switch_steps

        self.pool = Pool(n_cores)
        self.indexes = np.concatenate([range(self.n_classes) for __ in range(self.buffer_size//self.n_classes)])
        self.init_compl = Complicator_c(complexity,self.init_switch_steps)
        self.gradual_compl = Complicator_c_gradual(self.complexity, self.switch_steps)
        self.X = np.stack(self.pool.map(self.init_compl, self.indexes))
        self.y = np.zeros((buffer_size, self.n_classes), np.float32)
        self.y[range(buffer_size), self.indexes]=1.
        
    def __len__(self):
            return  self.buffer_size//self.batch_size
        
    def __getitem__(self, idx):
        batch_x_init = self.X[idx * self.batch_size:(idx + 1) * self.batch_size].copy()
        batch_y = self.y[idx * self.batch_size:(idx + 1) * self.batch_size]
        gradual_compl = Complicator_c_gradual(self.complexity, self.switch_steps)
        batch_x_mod = np.stack(self.pool.map(gradual_compl, list(batch_x_init)))
        self.X[idx * self.batch_size:(idx + 1) * self.batch_size] = batch_x_mod
        return batch_x_mod, batch_y
        
    
    
            

