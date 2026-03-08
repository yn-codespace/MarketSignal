import pandas   as pd
import numpy    as np
def read_csv(filename):

    # Read historic ticker data into pandas
    df = pd.read_csv(filename)

    return df


if __name__ == "__main__":
    print(' running from top-level module ')

    # Read in the data file
    # filename = 'stable_growth_df_finished.csv'
    filename = 'biggest_growth_df_finished.csv'
    df = read_csv(filename)
    # print(df.head(10))

    # Grab the column headers
    week_columns = df.columns[1:]  # skip first column
    print(week_columns)

    # Setup the loop 
    top_X = 500
    top_lists = []
    week_columns_concat = week_columns[:6]

    
    # Find the repeated tickers through set subtraction
    for col in week_columns_concat:  # or however many horizons you want
        top_df = df.sort_values(col, ascending=False).head(top_X)
        top_tickers = set(top_df["Ticker"].values)
        top_lists.append(top_tickers)


    # Only keep tickers that appear in EVERY horizon
    recurring_tickers = set.intersection(*top_lists)
    
    print(f"Tickers in top {top_X} across horizons{week_columns_concat} out of {len(df)} total tickers.  Found ({len(recurring_tickers)} total):")
    print(recurring_tickers)
