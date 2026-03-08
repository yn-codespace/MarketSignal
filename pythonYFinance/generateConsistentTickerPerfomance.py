import pandas   as pd
import numpy    as np

def read_tickers(filename):

    # Read tickers into a list
    with open(filename, "r") as file:
        tickers = [line.strip() for line in file]

    return tickers

def read_csv(filename):

    # Read historic ticker data into pandas
    df = pd.read_csv(filename)

    return df

def calc_relative_diffs(filename, ticker, delta_weeks):

    # Read in the .csv file
    df = read_csv(filename)

    deltas = np.array([])
    
    if df.empty:
        print("No data available for this ticker. Skipping.")
        return deltas

    # Screen for extremely low priced stocks
    if df["Close"].iloc[-1] < 10:
        return None

    # Screen for very low trading volumne
    avg_volume = df["Volume"].tail(30).mean()
    if avg_volume < 500000:
        return None

    # Screen for very short lived tickers
    if len(df) < 252:
        return None
    
    df["datetime_col"] = pd.to_datetime(df["Date"], utc=True)
    df = df.set_index("datetime_col").sort_index()

    # print(df.head())
    # print(df["Date"].iloc[0])
    # print(type(df["Date"].iloc[0]))

    
    # Pandas math to find the timestap associated with 'X' weeks in the past
    collection_date = df.index[-1]
    # print(f'collection_date = {collection_date}')

    # Create a dictionary for the Pandas dataframe
    row = {"Ticker" : ticker}
    for i in range(len(delta_weeks)-1):

        start_of_period = collection_date - pd.Timedelta(weeks=delta_weeks[i+1])
        end_of_period   = collection_date - pd.Timedelta(weeks=delta_weeks[i])
        # print(f'start_of_period = {start_of_period}')
        # print(f'end_of_period   = {end_of_period}')

        # Get the closing values at the nearest available timestamps
        start_val = df.iloc[df.index.get_indexer([start_of_period], method="pad")[0]]["Close"]
        end_val   = df.iloc[df.index.get_indexer([end_of_period], method="pad")[0]]["Close"]
        
        # Compute relative difference for this period
        row[f"{delta_weeks[i]}-{delta_weeks[i+1]}wk"] = (end_val - start_val) / start_val * 100
    
        
    return row


if __name__ == "__main__":
    print(' running from top-level module ')
    
    # Grab the tickers 
    filename = "../symbols/all_tickers.txt"
    tickers = read_tickers(filename)

    # Look at the delta b/w the weeks:
    # 4, 8, 12, 16, 20, 24, ...
    num_years = 1
    weeks_per_year = 52
    weeks_per_test = 4
    weeks_per_year_wrt_test = int(weeks_per_year / weeks_per_test)
  
    
    delta_weeks = [ (i)*weeks_per_test for i in range(weeks_per_test*weeks_per_year_wrt_test) ]
    print(delta_weeks)

    # Now, read in the historical stock data... 
    # We will go back 'X' number of weeks

    # Loop setup
    len_tickers = len(tickers)
    data_rows   = []
    for i, ticker in enumerate(tickers):
    
        # Filename of ticker csv file
        filename = f"./HistoricData5Years_Finished/{ticker}_stocks_data.csv"
        row = calc_relative_diffs(filename, ticker, delta_weeks)
        
        print(f"i = {i} out of {len_tickers-1}\nrow = {row}")
        
        # Screen for rows that have no meaning
        if row is None:
            print(f"i = {i} out of {len_tickers-1} is none... skipping.")
        else:
            print(f"i = {i} out of {len_tickers-1}\nrow = {row}")
            data_rows.append(row)

    # Build the DataFrame
    growth_df = pd.DataFrame(data_rows)


    # Ensure numeric columns are floats
    for col in growth_df.columns[1:]:
            growth_df[col] = growth_df[col].astype(float)


    growth_df.to_csv("stable_growth_df.csv", index=False, float_format="%.2f")
    







