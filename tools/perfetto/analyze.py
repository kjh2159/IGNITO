from perfetto.trace_processor import TraceProcessor
import pandas as pd

""""
[track_id]
"""
track_map = {
    # gpu
    "power.rails.gpu": 1,
    "power.rails.display": 21,

    # cpu
    "power.rails.cpu.big": 14,
    "power.rails.cpu.mid": 13,
    "power.rails.cpu.little": 15,

    # memory
    "power.rails.ddr.a": 3,
    "power.rails.ddr.b": 8,
    "power.rails.ddr.c": 2,
    "power.rails.memory.interface": 12,

    # low-dropout power domain
    "power.rails.ldo.main.a": 17,
    "power.rails.ldo.main.b": 19,
    "power.rails.ldo.sub": 7,
}

ids = list(track_map.values())
names = list(track_map.keys())

nano = 1E-9
micro = 1E-6
milli = 1E-3
tp = TraceProcessor('traces-lp25.perfetto-trace')

collection = pd.DataFrame()
# power collection with calculation
for i, n in zip(ids, names):
    # data access
    query = f"SELECT * FROM counter WHERE track_id={i} ORDER BY ts;"
    result = tp.query(query).as_pandas_dataframe()
    
    # data aggregation
    delta = pd.DataFrame()
    delta[f'dt_{i}'] = result['ts'].diff()
    delta[f'dt_{i}'] *= nano
    delta[f'dE_{i}'] = result['value'].diff()
    delta[f'dE_{i}'] *= micro
    delta = delta.tail(-2).reset_index(drop=True)

    # power calculation
    delta[n] = delta[f'dE_{i}']/delta[f'dt_{i}']
    collection = pd.concat([collection, delta], axis=1)

# save to csv
filename = "perfetto/output-lp25.csv"
collection.to_csv(filename, index=True)