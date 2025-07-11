import streamlit as st
import pandas as pd
import matplotlib.pyplot as plt
import glob
import os

st.title("Simple Stock Visualizer")

@st.cache_data
def load_data_from_folder(folder_path):
    """Load data from a specific folder"""
    pattern = os.path.join(folder_path, "*.csv")
    files = glob.glob(pattern)
    
    if not files:
        return None
    
    dfs = []
    for file in files:
        try:
            df = pd.read_csv(file)
            dfs.append(df)
        except Exception as e:
            st.warning(f"Could not load {file}: {e}")
    
    if not dfs:
        return None
    
    data = pd.concat(dfs, ignore_index=True)
    data['datetime'] = pd.to_datetime(data['datetime'])
    return data

def load_uploaded_data(uploaded_files):
    """Load data from uploaded files"""
    if not uploaded_files:
        return None
    
    dfs = []
    for file in uploaded_files:
        try:
            df = pd.read_csv(file)
            dfs.append(df)
        except Exception as e:
            st.error(f"Could not load {file.name}: {e}")
    
    if not dfs:
        return None
    
    data = pd.concat(dfs, ignore_index=True)
    data['datetime'] = pd.to_datetime(data['datetime'])
    return data

# Choose data source
data_source = st.radio("Choose how to load your data:", 
                      ["Upload CSV files", "Load from folder"])

data = None

if data_source == "Upload CSV files":
    uploaded_files = st.file_uploader(
        "Upload your CSV files", 
        type=['csv'], 
        accept_multiple_files=True,
        help="Select all CSV files containing your stock data"
    )
    
    if uploaded_files:
        data = load_uploaded_data(uploaded_files)
    else:
        st.info("👆 Please upload your CSV files to get started")

else:  # Load from folder
    folder_path = st.text_input(
        "Enter the folder path containing your CSV files:", 
        value="",
        help="Enter the full path to the folder containing your CSV files"
    )
    
    if folder_path:
        if os.path.exists(folder_path):
            data = load_data_from_folder(folder_path)
            if data is None:
                st.error(f"No CSV files found in: {folder_path}")
        else:
            st.error(f"Folder does not exist: {folder_path}")
    else:
        st.info("👆 Please enter a folder path to get started")

# Only proceed if we have data
if data is not None:
    if len(data) == 0:
        st.error("No data found in the loaded files")
    else:
        st.success(f"✅ Loaded {len(data)} records from {data['symbol'].nunique()} symbols")
        
        # Get unique symbols
        symbols = sorted(data['symbol'].unique())
        
        # Dropdown for symbol selection
        selected_symbol = st.selectbox("Select Stock Symbol:", symbols)
        
        # Filter data for selected symbol
        filtered_data = data[data['symbol'] == selected_symbol].copy()
        filtered_data = filtered_data.sort_values('datetime')
        
        if len(filtered_data) == 0:
            st.warning("No data found for selected symbol")
        else:
            # Display basic info
            col1, col2, col3 = st.columns(3)
            with col1:
                st.metric("Records", len(filtered_data))
            with col2:
                st.metric("Date Range", 
                         f"{filtered_data['datetime'].min().date()} to {filtered_data['datetime'].max().date()}")
            with col3:
                st.metric("Latest Close", f"${filtered_data['close'].iloc[-1]:.2f}")
            
            # Create the plot
            fig, ax = plt.subplots(figsize=(12, 6))
            ax.plot(filtered_data['datetime'], filtered_data['close'], linewidth=2)
            ax.set_title(f"{selected_symbol} - Close Price Over Time")
            ax.set_xlabel("Date")
            ax.set_ylabel("Price ($)")
            ax.grid(True, alpha=0.3)
            
            # Format x-axis for better date display
            plt.xticks(rotation=45)
            plt.tight_layout()
            
            st.pyplot(fig)
            
            # Show recent data
            st.subheader("Recent Data")
            st.dataframe(filtered_data.tail(10))
            
            # Download option
            csv = filtered_data.to_csv(index=False)
            st.download_button(
                label="Download filtered data as CSV",
                data=csv,
                file_name=f"{selected_symbol}_data.csv",
                mime="text/csv"
            )