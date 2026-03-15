import pandas as pd
import numpy as np


def read_tickers(filename):

    with open(filename, "r") as file:
        tickers = [line.strip() for line in file]

    return tickers


def read_csv(filename):

    df = pd.read_csv(filename)

    return df


def calc_trend_projection(df, ticker, lookback_days=252):

    if df.empty:
        return None

    # Price filter
    if df["Close"].iloc[-1] < 10:
        return None

    # Liquidity filter
    avg_volume = df["Volume"].tail(30).mean()
    if avg_volume < 500000:
        return None

    # Ensure enough history
    if len(df) < lookback_days:
        return None

    # Convert date column
    df["datetime_col"] = pd.to_datetime(df["Date"], utc=True)
    df = df.set_index("datetime_col").sort_index()

    # Only use recent window
    df = df.tail(lookback_days)
    # print(df)

    # Time axis
    t = np.arange(len(df))

    # Log(price)
    log_price = np.log(df["Close"].values)

    # Linear regression
    slope, intercept = np.polyfit(t, log_price, 1)

    # Predicted values
    predicted = slope * t + intercept

    # Compute R²
    ss_res = np.sum((log_price - predicted) ** 2)
    ss_tot = np.sum((log_price - np.mean(log_price)) ** 2)

    r2 = 1 - (ss_res / ss_tot)

    # Trend score
    trend_score = slope * r2

    # Optional annualized growth estimate
    annual_growth = slope * 252

    return {
        "Ticker": ticker,
        "slope": slope,
        "r2": r2,
        "trend_score": trend_score,
        "annual_growth_est": annual_growth
    }


if __name__ == "__main__":

    print("running from top-level module")

    filename = "../symbols/all_tickers.txt"
    tickers = read_tickers(filename)
    len_tickers = len(tickers)
    
    data_rows = []

    num_total_days  = 252
    stride          = 15
    num_strides     = int(num_total_days/stride)
    lookback_days   = [stride*(i+1) for i in range(num_strides)]

    for i,lookback_day_i in enumerate(lookback_days):
    
        for j, ticker in enumerate(tickers):
    
            filename    = f"./HistoricData5Years_Finished/{ticker}_stocks_data.csv"
            df          = read_csv(filename)
         
            row = calc_trend_projection(df,ticker, lookback_day_i)
    
            if row is not None:
                data_rows.append(row)
    
            print(f"i = {i} out of {num_strides} loopback days j = {j} out of {len_tickers-1} \nrow = {row}")
    
        trend_df = pd.DataFrame(data_rows)
    
        trend_df.sort_values("trend_score", ascending=False, inplace=True)
    
        trend_df.to_csv(f"trend_projection_look_back_{lookback_day_i}_df.csv", index=False, float_format="%.6f")







