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

    # Extract the most recent closing value
    collection_closing_val = df["Close"].iloc[-1]
    # print(f'collection_closing_val = {collection_closing_val}') 
    
    # Pandas math to find the timestap associated with 'X' weeks in the past
    collection_date = df.index[-1]
    # print(f'collection_date = {collection_date}')

    # Create a dictionary for the Pandas dataframe
    row = {"Ticker" : ticker}
    for weeks_i in delta_weeks:

        # How many calendar days are in 'X' weeks
        weeks_duration  = pd.Timedelta(weeks=weeks_i)
        # print(f'weeks_duration = {weeks_duration}')
    
        # Find the target timestamp
        target_time     = collection_date - weeks_duration
        # print(f'target_time = {target_time}')
    
        # Find the nearest location
        nearest_loc     = df.index.get_indexer([target_time], method="pad")[0]
        # print(f'nearest_loc = {nearest_loc}')
        
        # Get the closest timestamp value
        closest_timestamp   = df.index[nearest_loc]
        # print(f'closest_timestamp = {closest_timestamp}')
      
        # Get the entire row as a Series
        closest_row         = df.iloc[nearest_loc]
        # print(f'closest_row = {closest_row}')
    
        # Extract the closing val for 'X' weeks in the past
        closing_val_i = df["Close"].iloc[nearest_loc]
        # print(f'closing_val_i = {closing_val_i}')

        relative_diff = (collection_closing_val - closing_val_i ) / closing_val_i * 100
        
        row[f"{weeks_i}wk"] = relative_diff
        
    return row


if __name__ == "__main__":
    print(' running from top-level module ')
    
    # Grab the tickers 
    filename = "../symbols/all_tickers.txt"
    tickers = read_tickers(filename)

    # Look at the delta b/w the weeks:
    # 1, 2, 3, 4, 8, 13, ... then, j
    # by quqrter until the 5 year mark
    weeks_in_quarter = 13
    num_years = 5
    num_quarters = 4 * num_years
    temp_vec = [ weeks_in_quarter*(i+1) for i in range(num_quarters)]
    # print(temp_vec)

    delta_weeks = [ (i+1) for i in range(12) ]
    delta_weeks.extend(temp_vec)
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


    # # Sort by the largest horizon, e.g., 4-week growth
    # growth_df.sort_values("26wk", ascending=False, inplace=True)
    
    # print("Top fast-growing tickers over last 4 weeks:")
    # print(growth_df.head(10))

    growth_df.to_csv("growth_df.csv", index=False, float_format="%.2f")
    







