class AudioCategoriesController < ApplicationController
  def index
    list
    render :action => 'list'
  end

  # GETs should be safe (see http://www.w3.org/2001/tag/doc/whenToUseGet.html)
  verify :method => :post, :only => [ :destroy, :create, :update ],
         :redirect_to => { :action => :list }

  def list
    @audio_category_pages, @audio_categories = paginate :audio_categories, :per_page => 10
  end

  def show
    @audio_category = AudioCategory.find(params[:id])
  end

  def new
    @audio_category = AudioCategory.new
  end

  def create
    @audio_category = AudioCategory.new(params[:audio_category])
    if @audio_category.save
      flash[:notice] = 'AudioCategory was successfully created.'
      redirect_to :action => 'list'
    else
      render :action => 'new'
    end
  end

  def edit
    @audio_category = AudioCategory.find(params[:id])
  end

  def update
    @audio_category = AudioCategory.find(params[:id])
    if @audio_category.update_attributes(params[:audio_category])
      flash[:notice] = 'AudioCategory was successfully updated.'
      redirect_to :action => 'show', :id => @audio_category
    else
      render :action => 'edit'
    end
  end

  def destroy
    AudioCategory.find(params[:id]).destroy
    redirect_to :action => 'list'
  end
end
